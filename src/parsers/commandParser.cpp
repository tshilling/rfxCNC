#include "commandParser.h"
#define MAX_LINE_LENGTH 256

/*
NOT a terminal, needs to parse single letter to 8 letter commands
*/

namespace PARSER{
    String result_description[]={
        "Ok",
        "Zero length",
        "Too long",
        "Invalid token"
    };
    String end_of_line = ";/\n\r";

    result_struct parse(String input){
        result_struct result;
        result.status= ok;
        result.command = nullptr;

        // Check length of input and return if error
        if(input.length() == 0){
            result.status = zero_length;
            return result;
        }
        if(input.length() >= MAX_LINE_LENGTH){
            result.status = too_long;
            return result;
        }
        // Find the end of the input, but don't change the string
        int line_termination_index = input.length();
        for(uint8_t i = 0; i < end_of_line.length();i++){
            int index = input.indexOf(end_of_line[i]);
            // If character is present and it is less than any other termination character
            if(index >= 0 && index < line_termination_index)
                line_termination_index = index;
        }
        // Deal with comments and strip them out
        result.command = new command_struct();

        char commands[256];
        char comments[256];
        uint8_t commands_index = 0;
        uint8_t comments_index = 0;
        uint8_t comment_counter = 0;
        uint8_t whitespace_count = 0;
        for(uint8_t i = 0; i < line_termination_index; i++){
            if(input[i] == '('){
                comment_counter++;
                if(comment_counter == 1)
                    continue;
            }
            if(input[i] == ')'){
                comment_counter--;
                if(comment_counter == 0)
                    continue;
                // Unmatched ending comment, error
                if(comment_counter < 0){
                    result.status = invalid_token;
                    result.command = nullptr;
                    return result;
                }
            }
            if(comment_counter==0){
                if(input[i] == '\t')
                    input[i] = ' ';
                if(input[i]==' ')
                    whitespace_count++;
                else
                    whitespace_count=0;
                if(whitespace_count<=1)
                {
                    if(input[i] >= 'a' && input[i] <= 'z')
                        commands[commands_index++] = input[i]-32;
                    else
                        commands[commands_index++] = input[i];
                }
            }
            else
            {
                comments[comments_index++] = input[i];
            }
        }
        
        // Terminate strings
        commands[commands_index] = 0;
        comments[comments_index] = 0;

        result.command->input = String(commands);
        result.command->comment = String(comments);

        /*
            At this point the input line has been stripped of all comments, line terminations and delete blocks.
            All whitespaces have been condensed to a single space char including tabs and multiple spaces/tabs

            Seperation of command words, numbers and subsequent is defined by either a change of word/number or by space between words / numbers 
            LLL###LLL
            LLL ###
            LLL LLL
            ### ###
        */
        int index_of_last_start = 0;
        enum char_type_enum{
            number,
            character,
            space,
            not_defined
        };
        char_type_enum last_type = not_defined;
        char_type_enum current_type = not_defined;
        key_value_pair_struct pair;
        for(uint8_t i = 0; i < result.command->input.length();i++){
            char current = result.command->input[i];
            if((current >= '0' && current <= '9') || current == '.' || current == '-'|| current == '+'){
                // is a number
                current_type = number;
            }
            else if(current == ' '){
                // is not a word letter
                current_type = space;
            }
            else{
                current_type = character;
            }
            if(i!=0){
                if((current_type != last_type)){
                    if(last_type == number){
                        pair.value = result.command->input.substring(index_of_last_start,i).toFloat();
                        result.command->parameter.push_back(pair);
                        pair.key = "";
                        pair.value = NAN;
                    }
                    else{
                        if(pair.key.length() > 0)
                            if(pair.key[0]!= ' ')
                                result.command->parameter.push_back(pair);
                        pair.key = result.command->input.substring(index_of_last_start,i);
                        pair.value = NAN;
                    }
                    index_of_last_start = i;
                    last_type = current_type;
                }
            }
            else
            {
                last_type = current_type;
            }
            
        }
        if(current_type==character){
            pair.key = result.command->input.substring(index_of_last_start);
            pair.value = NAN;
            result.command->parameter.push_back(pair);
        }
        if(current_type==number){
            pair.value = result.command->input.substring(index_of_last_start).toFloat();
            result.command->parameter.push_back(pair);
        }
        return result;
    }
}