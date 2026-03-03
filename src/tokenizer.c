#include "tokenizer.h"
#define MAX_TOKEN_ARRAY_COUNT 128

static tokenizer_state *state;

bool is_digit(char *atom);
int  convert_to_digit(char *atom);
#define ERROR_REPORT(error, token, type)                                                                               \
    {                                                                                                                  \
        error_report(error, token, type);                                                                              \
        return false;                                                                                                  \
    }                                                                                                                  \
// error reproting: i dont know if this should be in the tokenizer file :)
typedef enum error_type
{
    GENERAL        = 0,
    INVALID_SYNTAX = 1,
} error_type;

void error_report(const char *error, token *token, error_type type)
{
    char *exp    = state->exp;
    int   length = state->exp_length;

    switch (type)
    {
        case GENERAL: {
            printf("%s\n", error);
        }
        break;
        case INVALID_SYNTAX: {
            int starting_ind = token->starting_ind;
            int ending_ind   = token->ending_ind;

            printf("%s: ", error);

            if (starting_ind != 0)
            {
                printf("%.*s", starting_ind, exp);
            }
            printf("\e[31m%.*s\e[0m", (ending_ind - starting_ind), &exp[starting_ind]);
            printf("%s\n", &exp[ending_ind]);
        }
        break;
    }
}

int token_get_precedence(token_type op)
{
    switch (op)
    {
        case TOKEN_MINUS:
            return 1;
        case TOKEN_PLUS:
            return 1;
        case TOKEN_MULTIPLICATION:
            return 2;
        case TOKEN_DIVISION:
            return 2;
        case TOKEN_EXPONENT:
            return 5;
        case TOKEN_OPEN_PAREN:
            return 0;
        case TOKEN_CLOSE_PAREN:
            return 6;
        default:
            return 0.0;
    }
    return INT_MIN;
}
// this is char* because we are still doing the lexial analysis part: we want to convert

bool char_is_operator(char *token)
{
    if (token == NULL)
    {
        printf("Expected an operator token but got NULL instead\n");
        DEBUG_BREAK;
    }
    switch (*token)
    {
        case TOKEN_MINUS:
        case TOKEN_PLUS:
        case TOKEN_MULTIPLICATION:
        case TOKEN_DIVISION:
        case TOKEN_EXPONENT:
        case TOKEN_OPEN_PAREN:
        case TOKEN_CLOSE_PAREN:
        case TOKEN_DECIMAL_POINT:
            return true;
        default:
            return false;
    }
    return false;
}

bool token_is_operator(token *token)
{
    return char_is_operator((char *)&token->type);
}

const char *token_get_string(token *token)
{
    if (token == NULL)
    {
        printf("Expected an operator token but got NULL instead\n");
        DEBUG_BREAK;
    }
    switch (token->type)
    {
        case TOKEN_INT:
            return "TOKEN_INT";
        case TOKEN_MINUS:
            return "TOKEN_MINUS";
        case TOKEN_PLUS:
            return "TOKEN_PLUS";
        case TOKEN_MULTIPLICATION:
            return "TOKEN_MULTIPLICATION";
        case TOKEN_DIVISION:
            return "TOKEN_DIVISION";
        case TOKEN_EXPONENT:
            return "TOKEN_EXPONENT";
        case TOKEN_OPEN_PAREN:
            return "TOKEN_OPEN_PAREN";
        case TOKEN_CLOSE_PAREN:
            return "TOKEN_CLOSE_PAREN";
        default:
            return "";
    }
    return "";
}

int token_get_atom()
{
    int    index = state->curr_token;
    token *token = &state->tokens[index];
    if (token->type == TOKEN_END)
    {
        return INT_MIN;
    }
    if (token->type != TOKEN_INT)
    {
        printf("Expected TOKEN_TYPE INT but got %s\n", token_get_string(token));
        DEBUG_BREAK;
    }
    int value = token->value;
    return value;
}

void token_consume()
{
    state->curr_token++;
}

token *token_peek()
{
    int index = state->curr_token;
    return &state->tokens[index];
}

token *token_peek_next()
{
    int index = state->curr_token;
    if (index + 1 > state->tokens_length)
    {
        // return 'END OF EXP' token;
        return &state->tokens[index];
    }
    return &state->tokens[index + 1];
}

bool init_tokenizer(arena *arena, char *exp)
{
    // if we have never initialized this then initialize it.
    if (exp == NULL)
    {
        printf("Expression is NULL\n");
        DEBUG_BREAK;
    }
    if (state == NULL)
    {
        state = arena_alloc(arena, sizeof(tokenizer_state));
    }
    state->exp           = exp;
    state->exp_length    = strlen(exp);
    state->curr_token    = 0;
    state->tokens_length = 0;

    // convert exp into tokens

    int index = 0;

    char *ptr = exp;

    // stage 1: process the exp into individual tokens

    token temp                               = {.type = TOKEN_END, .value = INT_MIN};
    token temp_tokens[MAX_TOKEN_ARRAY_COUNT] = {temp};

    do
    {
        int starting = ptr - exp;
        if (is_digit(ptr))
        {
            int ending = starting;

            int value = convert_to_digit(ptr);
            ptr++;

            while (ptr != NULL && *ptr != '\n' && *ptr != '\0' && is_digit(ptr))
            {
                value *= 10;
                value += convert_to_digit(ptr);
                ptr++;
                ending++;
            }
            temp.type         = TOKEN_INT;
            temp.value        = value;
            temp.starting_ind = starting;
            temp.ending_ind   = ending + 1;
        }
        else if (char_is_operator(ptr))
        {

            temp.type         = (token_type)*ptr;
            temp.value        = INT_MIN;
            temp.starting_ind = starting;
            temp.ending_ind   = starting + 1;
            ptr++;
        }
        else if (*ptr == ' ')
        {
            ptr++;
            continue;
        }
        else
        {
            ERROR_REPORT("Invalid exp", &(token){}, GENERAL);
        }
        temp_tokens[index++] = temp;
    } while (ptr != NULL && *ptr != '\0' && *ptr != '\n');

    // stage 2: check for floating point values, if there are merge them. And also check for wrong floating point tokens
    // for example a.b....c
    int len = index;
    token floating_point[MAX_TOKEN_ARRAY_COUNT] = {};
    
    if(temp_tokens[0].type == TOKEN_DECIMAL_POINT)
    {
        ERROR_REPORT("Misplaced decimal point", &temp_tokens[0], INVALID_SYNTAX); 
    }

    for (int i = 1; i <= len; i++)
    {

        token *curr_token = &temp_tokens[i];
        token *next_token = &temp_tokens[i + 1];
        token *prev_token = &temp_tokens[i - 1] ;

        if (curr_token->type == TOKEN_DECIMAL_POINT)
        {
            if(prev_token->type != TOKEN_INT || next_token->type != TOKEN_INT)
            {
                ERROR_REPORT("Misplaced decimal point", curr_token, INVALID_SYNTAX); 
            }
            char *end_ptr;
            float num = strtof(&state->exp[prev_token->starting_ind], &end_ptr);
            if(*end_ptr == TOKEN_DECIMAL_POINT)
            {
                ERROR_REPORT("Floating point error", curr_token, INVALID_SYNTAX);
            }

            prev_token->type = TOKEN_INT;
            prev_token->value = num;
            prev_token->ending_ind = next_token->ending_ind;
            
            //invalid the next two tokens
            curr_token->type = TOKEN_END;
            next_token->type = TOKEN_END;
        }
    }

    int floating_ind = 0;
    for (int i = 0; i <= len; i++)
    {
        token* curr_token = &temp_tokens[i];
        
        if(curr_token->type != TOKEN_END)
        {
            floating_point[floating_ind++] = temp_tokens[i];
        }
    }

    for (int i = 0; i <= floating_ind ; i++)
    {
        temp_tokens[i] = floating_point[i];
    }
    len = floating_ind; 
   
    // stage 3: check for syntax errors : I dont know if the tokenizer should be doing this but oh well. Im still
    // learning 02.03.2026 @GalacticPast

    int open_paren = 0;
    for (int i = 0; i <= len; i++)
    {
        token *curr_token       = &temp_tokens[i];
        token *next_token       = &temp_tokens[i + 1];
        bool   is_prev_operator = true;
        if (i != 0)
        {
            is_prev_operator = token_is_operator(&temp_tokens[i - 1]);
        }

        switch (curr_token->type)
        {
            case TOKEN_INT: {
                // consequetive tokens
                if (next_token->type == TOKEN_INT)
                {
                    // this means there was two consequetive integers seperated by a whitespace: for example 0 + 1 2 ...
                    // this is an error                                                                           ^
                    ERROR_REPORT("Invalid Expression: there was two consequetive integers", next_token, INVALID_SYNTAX);
                }
            }
            break;
            case TOKEN_MINUS: {
                // this means this is a unary operator
                // case when its not an error :
                //  -a - -b , -a--b
                if (next_token->type == TOKEN_INT)
                {
                    // for it to be a unary operator the previous token has to be an operator
                    if (state->exp[curr_token->ending_ind] != ' ' && is_prev_operator)
                    {
                        next_token->value *= -1;
                        // invalidate this token
                        curr_token->type   = TOKEN_END;
                        curr_token->value  = INT_MIN;
                        continue;
                    }
                    // this means we have an expression like this a -- b
                    else if (state->exp[curr_token->ending_ind] == ' ' && is_prev_operator)
                    {
                        ERROR_REPORT("Do you mean an unary operator? ", curr_token, INVALID_SYNTAX);
                        // if the previous token is not an operator dont do anything
                    }
                    else if (is_prev_operator == true && token_is_operator(next_token))
                    {
                        ERROR_REPORT("Consequetive tokens", curr_token, INVALID_SYNTAX);
                    }
                    // case when its an error:
                }
            }
            break;
            case TOKEN_PLUS: {
                // i mean this is uncesary but still
                if (next_token->type == TOKEN_INT)
                {
                    if (state->exp[curr_token->ending_ind] != ' ' && is_prev_operator)
                    {
                        // invalidate this token
                        curr_token->type  = TOKEN_END;
                        curr_token->value = INT_MIN;
                        continue;
                    }
                    // this means we have an expression like this a -- b
                    else if (state->exp[curr_token->ending_ind] == ' ' && is_prev_operator)
                    {
                        ERROR_REPORT("Do you mean an unary operator? ", curr_token, INVALID_SYNTAX);
                        return false;
                    }
                    // if the previous token is not an operator dont do anything
                }
                else if (is_prev_operator == true && token_is_operator(next_token))
                {
                    ERROR_REPORT("Consequetive tokens", curr_token, INVALID_SYNTAX);
                }
            }
            break;
            case TOKEN_DIVISION:
            case TOKEN_MULTIPLICATION: {
                if (is_prev_operator == true && next_token->type == TOKEN_INT)
                {
                    if (state->exp[curr_token->ending_ind] == ' ')
                    {
                        ERROR_REPORT("Consequetive tokens", curr_token, INVALID_SYNTAX);
                    }
                    else
                    {
                        ERROR_REPORT("Unary expression with this type not allowed.", curr_token, INVALID_SYNTAX);
                    }
                }
                else if (is_prev_operator == true && token_is_operator(next_token))
                {
                    ERROR_REPORT("Consequetive tokens", curr_token, INVALID_SYNTAX);
                }
            }
            break;
            case TOKEN_DECIMAL_POINT: {
            }
            break;
            case TOKEN_EXPONENT: {
            }
            break;
            case TOKEN_OPEN_PAREN: {
                open_paren++;
                if (is_prev_operator == false)
                {
                    ERROR_REPORT("Expected an operator before an open parenthesis", curr_token, INVALID_SYNTAX);
                }
                if (next_token->type == TOKEN_CLOSE_PAREN)
                {
                    ERROR_REPORT("Nothing inside the paraenthseis", next_token, INVALID_SYNTAX);
                }
            }
            break;
            case TOKEN_CLOSE_PAREN: {
                open_paren--;

                if (open_paren == -1)
                {
                    ERROR_REPORT("Misplaced Close Paren", curr_token, INVALID_SYNTAX);
                }
                if (next_token->type == TOKEN_OPEN_PAREN)
                {
                    ERROR_REPORT("Expected an operator", next_token, INVALID_SYNTAX);
                }
                else if (token_is_operator(next_token) == false && next_token->type != TOKEN_END)
                {
                    ERROR_REPORT("Expected an operator", curr_token, INVALID_SYNTAX);
                }
            }
            break;
            case TOKEN_END: {
                if (is_prev_operator && temp_tokens[i - 1].type != TOKEN_CLOSE_PAREN)
                {
                    ERROR_REPORT("There is no integer after this operator", &temp_tokens[i - 1], INVALID_SYNTAX);
                }
            }
            default:
                break;
        }
    }

    if (open_paren != 0)
    {
        ERROR_REPORT("Unclosed open Paren", &temp_tokens[len], INVALID_SYNTAX);
    }

    int final_tokens_index = 0;
    for (int i = 0; i < len; i++)
    {
        if (temp_tokens[i].type != TOKEN_END)
        {
            state->tokens[final_tokens_index++] = temp_tokens[i];
        }
    }
    state->tokens_length = final_tokens_index;
    return true;
}

bool is_digit(char *atom)
{
    if (atom == NULL)
    {
        printf("Expected an atom to check if it's digit but got NULL instead\n");
        DEBUG_BREAK;
    }
    bool ans = (*atom >= '0' && *atom <= '9' ? 1 : 0);
    return ans;
}

int convert_to_digit(char *atom)
{
    if (atom == NULL)
    {
        printf("Expected an digit atom but got NULL instead\n");
        DEBUG_BREAK;
    }
    if (is_digit(atom) == false)
    {
        printf("Expected the provided atom: %c, to be an digit.\n", *atom);
        DEBUG_BREAK;
    }
    int digit = *atom - '0';
    return digit;
}
