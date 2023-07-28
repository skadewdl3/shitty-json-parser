#include "./json-parser.h"
#include <stdio.h>

int main () {
  char json_string[] = "{\"name\":\"Soham K\", \"age\": 20, \"dumb\": true, \"knowledge\": null}";
  JSONTokens *result = parse_json(json_string);
  JSONToken* token = get_token("knowledge", result);
  printf("\n%p\n", parse_token_null(token, json_string));
  free(result);
}