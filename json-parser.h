#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <stdarg.h>

#define MAX_SIZE 100

#ifndef __bool_true_false_are_defined
#define true 1
#define false 0
typedef unsigned short int bool;
#endif

typedef enum
{
  JSON_BOOLEAN,
  JSON_INT,
  JSON_FLOAT,
  JSON_STRING,
  JSON_NULL,
  JSON_OBJECT,
  JSON_ARRAY
} SimpleJSONParser_TokenType;

typedef struct {
  SimpleJSONParser_TokenType type;
  int start;
  int end;
  char *name;
} JSONToken;

typedef struct
{
  JSONToken **tokens;
  int count;
} JSONTokens;

typedef enum
{
  ReadingKey,
  ReadingValue,
  ReadingArray,
  ExpectingKey,
  ExpectingValue,
  ExpectingColon,
  ExpectingComma,
  ExpectingBoolean,
  ExpectingNumber,
  ExpectingString,
  ExpectingObject,
  ExpectingArray,
  ExpectingNull,
  Error,
  None
} SimpleJSONParser_State;

bool is_number (char c) {
  return c >= 48 && c <= 57;
}

char* SimpleJSONParser_token_types[] = {"JSON_BOOLEAN", "JSON_INT", "JSON_FLOAT", "JSON_STRING", "JSON_NULL"};
char *SimpleJSONParser_states[] = {"ReadingKey", "ReadingValue", "ReadingArray", "ExpectingKey", "ExpectingValue", "ExpectingColon", "ExpectingComma", "ExpectingBoolean", "ExpectingNumber", "ExpectingString", "ExpectingObject", "ExpectingArray", "ExpectingNull", "Error", "None"};

int SimpleJSONParser_tokenize(char *string, int start, int end, JSONTokens *result, int tokens_length)
{
  
  SimpleJSONParser_State state = None;
  SimpleJSONParser_State substate = ExpectingKey;
  JSONToken* token = NULL;

  int key_start = 0;
  int key_end = 0;

  int i = start;

  void SimpleJSONParser_add_token(JSONToken * token)
  {
    if (result->tokens == NULL)
      result->tokens = (JSONToken **)malloc(sizeof(JSONToken *));
    else
      result->tokens = (JSONToken **)realloc(result->tokens, (result->count + 1) * sizeof(JSONToken *));
    result->tokens[result->count] = token;
    result->count += 1;
  }

  void SimpleJSONParser_increment_i(int val)
  {
    #ifdef ENABLE_JSON_DEBUGGING
    printf("%c: %s - %s\n", string[i], SimpleJSONParser_states[state], SimpleJSONParser_states[substate]);
    #endif
    i += val;
  }

  JSONToken *SimpleJSONParser_create_token(int start, int end, SimpleJSONParser_TokenType type)
  {
    char *name = NULL;
    if (key_start == -1 && key_end == -1) {
      int digits = (int)log10(result->count + 1) + 1;
      name = (char *)malloc((digits + 1) * sizeof(char));
    }
    else {
      name = (char *)malloc((key_end - key_start + 2) * sizeof(char));
    }
    JSONToken *token = (JSONToken *)malloc(sizeof(JSONToken));
    token->start = start;
    token->end = end;
    token->type = type;
    token->name = name;
    int x = 0;
    if (key_start == -1 && key_end == -1) {
      int digits = (int)log10(result->count + 1) + 1;
      sprintf(token->name, "%d", result->count);
      token->name[digits] = '\0';
    }
    else {  
      for (int j = key_start; j <= key_end; j++, x++)
      {
        token->name[x] = string[j];
      }
      token->name[key_end - key_start + 1] = '\0';
    }
    return token;
  }

  void handle_number()
  {
    int num_start = i;
    bool is_float = false;
    while (1)
    {
      if (string[i] == '.')
        is_float = true;
      if (string[i + 1] == ',')
        substate = ExpectingComma;
      if (string[i] == ',' || string[i] == ']' || string[i] == '}')
      {
        if (state != ReadingArray) {
          state = None;
          substate = ExpectingKey;
        }
        else {
          state = ReadingArray;
          substate = ExpectingComma;
        }
        break;
      }
      SimpleJSONParser_increment_i(1);
    }
    JSONToken *num_token = NULL;
    if (is_float)
      num_token = SimpleJSONParser_create_token(num_start, i - 1, JSON_FLOAT);
    else
      num_token = SimpleJSONParser_create_token(num_start, i - 1, JSON_INT);
    SimpleJSONParser_add_token(num_token);
  }

  void handle_string()
  {
    SimpleJSONParser_increment_i(1);
    int str_start = i;
    while (1)
    {
      if (string[i] == '"' && string[i - 1] != '\\')
      {
        if (state != ReadingArray) state = None;
        substate = ExpectingComma;
        break;
      }
      SimpleJSONParser_increment_i(1);
    }
    JSONToken *token = SimpleJSONParser_create_token(str_start, i - 1, JSON_STRING);
    SimpleJSONParser_add_token(token);
  }

  void handle_boolean()
  {
    if (string[i] == 't' && string[i + 1] == 'r' && string[i + 2] == 'u' && string[i + 3] == 'e')
    {
      JSONToken *token = SimpleJSONParser_create_token(i, i + 3, JSON_BOOLEAN);
      SimpleJSONParser_add_token(token);
      if (state != ReadingArray) state = None;
      substate = ExpectingComma;
      SimpleJSONParser_increment_i(3);
    }
    else if (string[i] == 'f' && string[i + 1] == 'a' && string[i + 2] == 'l' && string[i + 3] == 's' && string[i + 4] == 'e')
    {
      JSONToken *token = SimpleJSONParser_create_token(i, i + 4, JSON_BOOLEAN);
      SimpleJSONParser_add_token(token);
      if (state != ReadingArray)
        state = None;
      substate = ExpectingComma;
      SimpleJSONParser_increment_i(4);
    }
  }

  void handle_null()
  {
    if (string[i] == 'n' && string[i + 1] == 'u' && string[i + 2] == 'l' && string[i + 3] == 'l')
    {
      JSONToken *token = SimpleJSONParser_create_token(i, i + 3, JSON_NULL);
      SimpleJSONParser_add_token(token);
      if (state != ReadingArray)
        state = None;
      substate = ExpectingComma;
      SimpleJSONParser_increment_i(3);
    }
  }

  void handle_object () {
    int obj_start = i;
    while (1) {
      if (string[i] == '}') {
        JSONToken *token = SimpleJSONParser_create_token(obj_start, i, JSON_OBJECT);
        SimpleJSONParser_add_token(token);
        if (state != ReadingArray)
          state = None;
        substate = ExpectingComma;
        break;
      }
      SimpleJSONParser_increment_i(1);
    }
  }

  void handle_array () {
    int arr_start = i;
    while (1) {
      if (string[i] == ']') {
        JSONToken *token = SimpleJSONParser_create_token(arr_start, i, JSON_ARRAY);
        SimpleJSONParser_add_token(token);
        if (state != ReadingArray)
          state = None;

        substate = ExpectingComma;
        break;
      }
      SimpleJSONParser_increment_i(1);
    } 
  }

  for (; i <= end; SimpleJSONParser_increment_i(1))
  {
    char c = string[i];

    if (c == ' ' && state != ReadingValue && state != ReadingArray) continue;

    if (c == '"') {
      if (substate == ExpectingKey) {
        key_start = i + 1;
        state = ReadingKey;
        substate = None;
      }
      else if (state == ReadingKey) {
        key_end = i - 1;
        state = None;
        substate = ExpectingColon;
      }
      else if (substate == ExpectingValue) {
        state = ReadingValue;
        substate = ExpectingString;
        handle_string();
      }
      else if (state == ReadingValue && substate == ExpectingString) {
        state = None;
        substate = ExpectingComma;
      }
    }

    if (c == ':' && substate == ExpectingColon) {
      substate = ExpectingValue;
    }

    if (c == ',' && state != ReadingArray && substate == ExpectingComma) {
      substate = ExpectingKey;
    }

    if (c == '[' && state == None && substate == ExpectingKey && result->count == 0)
    {
      state = ReadingArray;
      substate = None; 
      SimpleJSONParser_increment_i(1);
      c = string[i];
    }

    if (substate == ExpectingValue) {
      if (c == 't' || c == 'f') {
        state = ReadingValue;
        substate = ExpectingBoolean;
        handle_boolean();
      }
      else if (c == 'n')
      {
        state = ReadingValue;
        substate = ExpectingNull;
        handle_null();
      }
      else if (is_number(c) || (c == '-' && is_number(string[i + 1]))) {
        state = ReadingValue;
        substate = ExpectingNumber;
        handle_number();
      }
      else if (c == '{') {
        state = ReadingValue;
        substate = ExpectingObject;
        handle_object();
      }
      else if (c == '[') {
        state = ReadingValue,
        substate = ExpectingArray;
        handle_array();
      }
    }
  
    if (state == ReadingArray) {
      if (c == ' ' && state != ExpectingString) {
        continue;
      }
      if (c == '"') {
        if (substate == ExpectingComma) {
          state = ReadingArray;
          substate = ExpectingString;
          handle_string();
        }
      }
      if (c == 't' || c == 'f') {
        state = ReadingArray;
        substate = ExpectingBoolean;
        key_start = -1;
        key_end = -1;
        handle_boolean();
      }
      else if (c == 'n')
      {
        state = ReadingArray;
        substate = ExpectingNull;
        key_start = -1;
        key_end = -1;
        handle_null();
      }
      else if (is_number(c) || (c == '-' && is_number(string[i + 1]))) {
        state = ReadingArray;
        substate = ExpectingNumber;
        key_start = -1;
        key_end = -1;
        handle_number();
      }
      else if (c == '{') {
        state = ReadingArray;
        substate = ExpectingObject;
        key_start = -1;
        key_end = -1;
        handle_object();
      }
      else if (c == '[') {
        state = ReadingArray,
        substate = ExpectingArray;
        key_start = -1;
        key_end = -1;
        handle_array();
      }
      else if (c == ']') {
        state = None;
        substate = ExpectingComma;
      }
    }
  }
}

JSONTokens *parse_json(char *string)
{
  if (string == NULL) return NULL;
  JSONTokens *result = (JSONTokens *)malloc(sizeof(JSONTokens));
  result->tokens = NULL;
  result->count = 0;
  SimpleJSONParser_tokenize(string, 0, strlen(string) - 1, result, 0);
  return result;
}

JSONTokens* parse_json_with_limits(char* string, int start, int end) {
  if (string == NULL) return NULL;
  JSONTokens *result = (JSONTokens *)malloc(sizeof(JSONTokens));
  result->tokens = NULL;
  result->count = 0;
  SimpleJSONParser_tokenize(string, start, end, result, 0);
  return result;
}


char *parse_token(JSONToken *token, char *string)
{
  char *temp = (char *)malloc((token->end - token->start + 2) * sizeof(char));
  for (int i = token->start, x = 0; i <= token->end; i++, x++)
  {
    temp[x] = string[i];
  }
  temp[token->end - token->start + 1] = '\0';
  return temp;
}

char *parse_token_str(JSONToken* token, char* json_string) {
  if (token->type != JSON_STRING) {
    printf("Ivalid JSON Token Type. Expected JSON_STRING. Got %s", SimpleJSONParser_token_types[token->type]);
    return NULL;
  }
  return parse_token(token, json_string);
}

char parse_token_char(JSONToken* token, char* json_string) {
  if (token->type != JSON_STRING) {
    printf("Ivalid JSON Token Type. Expected JSON_STRING. Got %s", SimpleJSONParser_token_types[token->type]);
    return '\0';
  }
  return parse_token(token, json_string)[0];
}

JSONTokens* parse_token_obj(JSONToken* token, char* json_string) {
  if (token->type != JSON_OBJECT) {
    printf("Ivalid JSON Token Type. Expected JSON_OBJECT. Got %s", SimpleJSONParser_token_types[token->type]);
    return NULL;
  }
  return parse_json_with_limits(json_string, token->start, token->end);
}

JSONTokens *parse_token_arr(JSONToken *token, char *json_string)
{
  if (token->type != JSON_ARRAY)
  {
    printf("Ivalid JSON Token Type. Expected JSON_ARRAY. Got %s", SimpleJSONParser_token_types[token->type]);
    return NULL;
  }
  return parse_json_with_limits(json_string, token->start, token->end);
}

int parse_token_int(JSONToken* token, char* json_string)
{
 if (token->type != JSON_INT) {
    printf("Ivalid JSON Token Type. Expected JSON_INT. Got %s", SimpleJSONParser_token_types[token->type]);
    return -1;
  }
  return atoi(parse_token(token, json_string));
}

float parse_token_float(JSONToken* token, char* json_string)
{
 if (token->type != JSON_FLOAT) {
    printf("Ivalid JSON Token Type. Expected JSON_FLOAT. Got %s", SimpleJSONParser_token_types[token->type]);
    return -1.0;
  }
  return atof(parse_token(token, json_string));
}

bool parse_token_bool(JSONToken* token, char* json_string)
{
 if (token->type != JSON_BOOLEAN) {
    printf("Ivalid JSON Token Type. Expected JSON_BOOLEAN. Got %s", SimpleJSONParser_token_types[token->type]);
    return false;
  }
  return strcmp(parse_token(token, json_string), "true") == 0;
}

void* parse_token_null(JSONToken* token, char* json_string)
{
 if (token->type != JSON_NULL) {
    printf("Ivalid JSON Token Type. Expected JSON_NULL. Got %s", SimpleJSONParser_token_types[token->type]);
    return NULL;
  }
  return NULL;
}


JSONToken* get_token_by_key (char* name, JSONTokens* result) {
  for (int i = 0; i < result->count; i++) {
    if (strcmp(name, result->tokens[i]->name) == 0) {
      return result->tokens[i];
    }
  }
  return NULL;
}

JSONToken *get_token_by_index(int index, JSONTokens *result)
{
  int digits = (int)log10(result->count) + 1;
  char name[digits + 1];
  sprintf(name, "%d", index);
  name[digits] = '\0';
  for (int i = 0; i < result->count; i++)
  {

    if (strcmp(name, result->tokens[i]->name) == 0)
    {
      return result->tokens[i];
    }
  }
  return NULL;
}

int get_value_by_key_int (char* name, JSONTokens* result, char* json_string) {
  JSONToken* token = get_token_by_key(name, result);
  return parse_token_int(token, json_string);
}

float get_value_by_key_float(char *name, JSONTokens *result, char* json_string)
{
  JSONToken *token = get_token_by_key(name, result);
  if (token->type == JSON_INT) return 1.0 * parse_token_int(token, json_string);
  return parse_token_float(token, json_string);
}

char* get_value_by_key_str(char *name, JSONTokens *result, char* json_string)
{
  JSONToken *token = get_token_by_key(name, result);
  return parse_token_str(token, json_string);
}

int get_value_by_key_bool(char *name, JSONTokens *result, char* json_string)
{
  JSONToken *token = get_token_by_key(name, result);
  return parse_token_bool(token, json_string);
}

char get_value_by_key_char(char *name, JSONTokens *result, char* json_string)
{
  JSONToken *token = get_token_by_key(name, result);
  return parse_token_char(token, json_string);
}

void* get_value_by_key_null(char *name, JSONTokens *result, char* json_string)
{
  JSONToken *token = get_token_by_key(name, result);
  return NULL;
}

JSONTokens* get_value_by_key_obj(char *name, JSONTokens *result, char* json_string)
{
  JSONToken *token = get_token_by_key(name, result);
  return parse_token_obj(token, json_string);
}

JSONTokens* get_value_by_key_arr(char *name, JSONTokens *result, char* json_string)
{
  JSONToken *token = get_token_by_key(name, result);
  return parse_token_arr(token, json_string);
}

int get_value_by_index_int(int index, JSONTokens *result, char* json_string)
{
  JSONToken *token = get_token_by_index(index, result);
  return parse_token_int(token, json_string);
}

float get_value_by_index_float(int index, JSONTokens *result, char* json_string)
{
  JSONToken *token = get_token_by_index(index, result);
  if (token->type == JSON_INT) return 1.0 * parse_token_int(token, json_string);
  return parse_token_float(token, json_string);
}

char *get_value_by_index_str(int index, JSONTokens *result, char* json_string)
{
  JSONToken *token = get_token_by_index(index, result);
  return parse_token_str(token, json_string);
}

int get_value_by_index_bool(int index, JSONTokens *result, char* json_string)
{
  JSONToken *token = get_token_by_index(index, result);
  return parse_token_bool(token, json_string);
}

char get_value_by_index_char(int index, JSONTokens *result, char* json_string)
{
  JSONToken *token = get_token_by_index(index, result);
  return parse_token_char(token, json_string);
}

void *get_value_by_index_null(int index, JSONTokens *result, char* json_string)
{
  JSONToken *token = get_token_by_index(index, result);
  return NULL;
}

JSONTokens *get_value_by_index_obj(int index, JSONTokens *result, char* json_string)
{
  JSONToken *token = get_token_by_index(index, result);
  return parse_token_obj(token, json_string);
}

JSONTokens *get_value_by_index_arr(int index, JSONTokens *result, char* json_string)
{
  JSONToken *token = get_token_by_index(index, result);
  return parse_token_arr(token, json_string);
}

char* format_json (char* format_string, ...) {
  char* temp = (char*)malloc(sizeof(char) * 1000);
  va_list arguments;
  va_start(arguments, format_string);
  vsprintf(temp, format_string, arguments);
  va_end(arguments);
  size_t length = strlen(temp);
  temp = realloc(temp, sizeof(char) * (length + 1));
  temp[length] = '\0';
  return temp;
}

// done
#define get_value_int(name, result, json_string) _Generic((name), \
    int: get_value_by_index_int,                                  \
    char *: get_value_by_key_int)(name, result, json_string)

#define get_value_float(name, result, json_string) _Generic((name), \
    int: get_value_by_index_float,                         \
    char*: get_value_by_key_float                     \
)(name, result, json_string)

#define get_value_str(name, result, json_string) _Generic((name), \
    int: get_value_by_index_str,                     \
    char *: get_value_by_key_str)(name, result, json_string)

#define get_value_bool(name, result, json_string) _Generic((name), \
    int: get_value_by_index_bool,                     \
    char *: get_value_by_key_bool)(name, result, json_string)

#define get_value_char(name, result, json_string) _Generic((name), \
    int: get_value_by_index_char,                     \
    char *: get_value_by_key_char)(name, result, json_string)

#define get_value_null(name, result, json_string) _Generic((name), \
    int: get_value_by_index_null,                     \
    char *: get_value_by_key_int)(name, result, json_string)

#define get_value_obj(name, result, json_string) _Generic((name), \
    int: get_value_by_index_obj,                     \
    char *: get_value_by_key_obj)(name, result, json_string)

#define get_value_arr(name, result, json_string) _Generic((name), \
    int: get_value_by_index_arr,                     \
    char *: get_value_by_key_arr)(name, result, json_string)
