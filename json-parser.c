#include "./json-parser.h"
#include <stdio.h>

int main () {
  char json_string[] = "{\"name\":\"Soham K\", \"dob\": {\"date\": 11, \"month\": \"June\", \"year\": 2004}, \"dumb\": true, \"knowledge\": null, \"marks\": [10, 20, 30]}";
  char json_string2[] = "[10, \"Soham\", true, null]";
  
  JSONTokens* result = parse_json(json_string2);

  printf("JSON Tokens:\n");
  printf("%d\n", parse_token_int(get_token(0, result), json_string2));
  printf("%s\n", parse_token_str(get_token(1, result), json_string2));
  printf("%d\n", parse_token_bool(get_token(2, result), json_string2));
  printf("%s\n", parse_token_null(get_token(3, result), json_string2));

  free(result);
}