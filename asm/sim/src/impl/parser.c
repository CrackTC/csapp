#include "parser.h"
#include "common.h"
#include "inst.h"
#include "parse/node.h"
#include "parse/parser.h"
#include "parse/parsers.h"
#include "reg.h"
#include "utils.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define DEFINE_PARSER(_, NAME) parse_parser_t *NAME;

#define INIT_INST_PARSER(P, I)                                                 \
  P->I = new_sequence_parser(2, new_lowercase_parser(#I),                      \
                             new_peek_parser(new_not_parser(alpha)));
#define INIT_REG_PARSER(P, R)                                                  \
  P->R = new_sequence_parser(2, new_string_parser(#R),                         \
                             new_peek_parser(new_not_parser(alpha)));

#define MAKE_STRING_ARG(_, A) MAKE_ARG(_, #A)
#define MAKE_PARSER_ARG(P, N) , (P)->N
#define COUNT_ARGS(first, ...)                                                 \
  (sizeof((const char *[]){__VA_ARGS__}) / sizeof(const char *))
#define INIT_NAME_PARSER(P, NAME, FOREACH)                                     \
  P->NAME = new_one_of_parser(EXPAND(COUNT_ARGS, FOREACH(MAKE_STRING_ARG))     \
                                  FOREACH(MAKE_PARSER_ARG, P));

struct parser_t {
  FOREACH_OP(DEFINE_PARSER)
  parse_parser_t *op;

  FOREACH_REG(DEFINE_PARSER)
  parse_parser_t *reg;
  parse_parser_t *reg_full;

  parse_parser_t *hex;
  parse_parser_t *imm;
  parse_parser_t *effective;

  parse_parser_t *inst;
};

void free_parser(parser_t *parser) {
  free_parse_parser(parser->inst);
  free(parser);
}

DEFINE_CLEANUP_FUNC(parser)

parser_t *new_parser() {
  parser_t *result = malloc(sizeof(parser_t));

  parse_parser_t *alpha = new_range_parser('a', 'z');
  FOREACH_OP(INIT_INST_PARSER, result)
  INIT_NAME_PARSER(result, op, FOREACH_OP);

  FOREACH_REG(INIT_REG_PARSER, result)
  INIT_NAME_PARSER(result, reg, FOREACH_REG);
  result->reg_full = new_sequence_parser(2, new_exact_parser('%'), result->reg);

  parse_parser_t *zero_to_nine = new_range_parser('0', '9');
  parse_parser_t *dec =
      new_sequence_parser(2, new_optional_parser(new_exact_parser('-')),
                          new_multiple_parser(zero_to_nine));
  result->hex = new_sequence_parser(
      3, new_optional_parser(new_exact_parser('-')), new_string_parser("0x"),
      new_multiple_parser(new_one_of_parser(3, new_range_parser('A', 'F'),
                                            new_range_parser('a', 'f'),
                                            zero_to_nine)));
  result->imm = new_sequence_parser(2, new_exact_parser('$'),
                                    new_one_of_parser(2, result->hex, dec));

  parse_parser_t *delim = new_sequence_parser(
      3, new_optional_parser(new_multiple_parser(new_exact_parser(' '))),
      new_exact_parser(','),
      new_optional_parser(new_multiple_parser(new_exact_parser(' '))));

  parse_parser_t *paren_parser = new_sequence_parser(
      5, new_exact_parser('('), new_optional_parser(result->reg_full),
      new_optional_parser(new_sequence_parser(2, delim, result->reg_full)),
      new_optional_parser(new_sequence_parser(
          2, delim, new_one_of_parser(2, result->hex, dec))),
      new_exact_parser(')'));

  result->effective = new_sequence_parser(
      2, new_optional_parser(new_one_of_parser(2, result->hex, dec)),
      new_optional_parser(paren_parser));

  parse_parser_t *operand_parser =
      new_one_of_parser(3, result->reg_full, result->imm, result->effective);

  result->inst = new_sequence_parser(
      3, result->op,
      new_optional_parser(new_sequence_parser(
          2, new_multiple_parser(new_exact_parser(' ')), operand_parser)),
      new_optional_parser(new_sequence_parser(2, delim, operand_parser)));

  return result;
}

#define CASE_OP(NODE, PARSER, OP)                                              \
  if ((NODE)->parser_ref == (PARSER)->OP) {                                    \
    return OP;                                                                 \
  }

static op_t parse_op(parser_t *parser, parse_node_t *node) {
  FOREACH_OP(CASE_OP, node, parser)
  assert(0 && "Unknown instruction");
}

static int64_t parse_dec_number(parse_node_t *node) {
  int sign = 1;
  if (node->children[0]->parser_ref != none_parser()) {
    sign = -1;
  }

  uint64_t result = 0;
  parse_node_t *digits = node->children[1];
  for (const char *ch = digits->start_ref; ch != digits->end_ref; ++ch) {
    result = (result << 3U) + (result << 1U) + *ch - '0';
  }

  return sign * (int64_t)result;
}

static int64_t parse_hex_number(parse_node_t *node) {
  int sign = 1;
  if (node->children[0]->parser_ref != none_parser()) {
    sign = -1;
  }

  uint64_t result = 0;
  parse_node_t *digits = node->children[2];
  for (const char *ch = digits->start_ref; ch != digits->end_ref; ++ch) {
    result <<= 4U;
    if ('0' <= *ch && *ch <= '9') {
      result += *ch - '0';
    } else if ('A' <= *ch && *ch <= 'F') {
      result += *ch - 'A' + 10;
    } else {
      result += *ch - 'a' + 10;
    }
  }

  return sign * (int64_t)result;
}

static int64_t parse_number(parser_t *parser, parse_node_t *node) {
  if (node->parser_ref == parser->hex) {
    return parse_hex_number(node);
  }
  return parse_dec_number(node);
}

#define CASE_REG(NODE, PARSER, REG)                                            \
  if ((NODE)->parser_ref == (PARSER)->REG) {                                   \
    *offset = OFFSET(REG);                                                     \
    if (mask != NULL)                                                          \
      *mask = REG_MASK(REG);                                                   \
    return;                                                                    \
  }

static void parse_reg_full(parser_t *parser, parse_node_t *node, size_t *offset,
                           uint64_t *mask) {
  parse_node_t *reg = node->children[1];
  FOREACH_REG(CASE_REG, reg, parser)
  assert(0 && "Unknown register");
}

static void parse_paren(parser_t *parser, parse_node_t *node, od_t *result) {
  if (node->children[1]->parser_ref != none_parser()) {
    result->type |= REG1;
    result->reg1_mask = 0;
    parse_reg_full(parser, node->children[1], &result->reg1_offset, NULL);
  }
  if (node->children[2]->parser_ref != none_parser()) {
    result->type |= REG2;
    parse_reg_full(parser, node->children[2]->children[1], &result->reg2_offset,
                   NULL);
  }
  if (node->children[3]->parser_ref != none_parser()) {
    result->type |= SCAL;
    result->scal = parse_number(parser, node->children[3]->children[1]);
  }
}

static od_t parse_effective(parser_t *parser, parse_node_t *node) {
  od_t result = {};
  result.type |= MM;
  parse_node_t *disp = node->children[0];
  parse_node_t *paren = node->children[1];
  if (disp->parser_ref != none_parser()) {
    result.type |= IMM;
    result.imm = parse_number(parser, disp);
  }
  if (paren->parser_ref != none_parser()) {
    parse_paren(parser, paren, &result);
  }
  return result;
}

static od_t parse_operand(parser_t *parser, parse_node_t *node) {
  if (node->parser_ref == parser->effective) {
    return parse_effective(parser, node);
  }

  if (node->parser_ref == parser->reg_full) {
    od_t result = {.type = REG};
    parse_reg_full(parser, node, &result.reg1_offset, &result.reg1_mask);
    return result;
  }

  if (node->parser_ref == parser->imm) {
    od_t result = {.type = IMM};
    result.imm = parse_number(parser, node->children[1]);
    return result;
  }

  assert(0 && "Unknown operand");
}

static od_t parse_src(parser_t *parser, parse_node_t *node) {
  if (node->parser_ref == none_parser()) {
    return (od_t){};
  }

  parse_node_t *operand = node->children[1];
  return parse_operand(parser, operand);
}

static od_t parse_dst(parser_t *parser, parse_node_t *node) {
  if (node->parser_ref == none_parser()) {
    return (od_t){};
  }

  parse_node_t *operand = node->children[1];
  return parse_operand(parser, operand);
}

inst_t *parser_parse_inst(parser_t *parser, const char *line) {
  parse_node_t *node = parser_parse(parser->inst, line);

  if (node == NULL) {
    return NULL;
  }

  inst_t *result = malloc(sizeof(inst_t));

  parse_node_t *op = node->children[0];
  result->op = parse_op(parser, op);

  parse_node_t *src = node->children[1];
  result->src = parse_src(parser, src);

  parse_node_t *dst = node->children[2];
  result->dst = parse_dst(parser, dst);

  free_parse_node(node);
  result->code = line;

  return result;
}
