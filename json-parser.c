#include "./json-parser.h"
#include <stdio.h>

int main () {
  char json_string2[] = "{\"operator\": \"+\", \"operand1\": 2.3, \"operand2\": 4.6}";
  
  JSONTokens* result = parse_json(json_string2);

  printf("\n%f", parse_token_float(get_token("operand1", result), json_string2));

  free(result);
}