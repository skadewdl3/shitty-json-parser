#include "./json-parser.h"
#include <stdio.h>

int main () {
  char json_string[] = "{\"name\":\"Soham K\", \"dob\": {\"date\": 11, \"month\": \"June\", \"year\": 2004}, \"dumb\": true, \"knowledge\": null, \"marks\": [3.14e-2, 20, 30]}";

  JSONTokens* result = parse_json(json_string);

  JSONTokens* dob = get_value_obj("dob", result, json_string);


  char* name = get_value_str("name", result, json_string);
  int day = get_value_int("date", dob, json_string);
  char* month = get_value_str("month", dob, json_string);
  int year = get_value_int("year", dob, json_string);
  printf("%s was born on %d %s %d.\n", name, day, month, year);


  free(name);
  free(month);
  free(result);
}

