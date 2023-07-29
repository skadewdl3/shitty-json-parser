#include "./json-parser.h"
#include <stdio.h>

int main () {
  char json_string[] = "{\"name\":\"Soham K\", \"dob\": {\"date\": 11, \"month\": \"June\", \"year\": 2004}, \"dumb\": true, \"knowledge\": null, \"marks\": [3.14e-2, 20, 30]}";

  JSONTokens* result = parse_json(json_string);

  JSONTokens* marks = get_value_arr("marks", result, json_string);

  for (int i = 0; i < marks->count; i++) {
    printf("%f\n", get_value_float(i, marks, json_string));
  }

  free(result);
}