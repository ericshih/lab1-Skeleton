// UCLA CS 111 Lab 1 command reading

#include "command.h"
#include "command-internals.h"

#include <error.h>
#include <stdio.h>
#include <string.h>

#include "alloc.h"

/* FIXME: You may need to add #include directives, macro definitions,
   static function definitions, etc.  */

command_stream_t proc_Buf_caller(char** buf);
command_t process_buffer(char* buf);
void end_word(bool* input_flag, bool* output_flag, command_t entry,
	      char* subset, int* word_count); 
bool analyze_token(char* token); //returns false if token ends with special 
				 //characters, e.g. a ||
void output_error(char* buffer, char* token, int pos); 

void reorder_commands(command_stream_t s);
//outputs error with line number found at token


struct command_stream
{
  command_t* my_commands;
  int next_pos;
  int size;
  //int (*func) (void*); //function pointer
  //void* v;	       //void *
};

void output_error(char* buffer, char* token, int pos)
{
  char* start = buffer;
  int i =0;

  int line_num = 1;
  if(pos == 0)
  {
	while(buffer[i] != '\0')
	{   
	   if(buffer[i]=='\n')
	     line_num++;
	   else if(buffer[i]=='\0')
	     break;
  	   //printf("%d %d\n",(int) buffer[i] , line_num);
	   i++;
	}
	error(1,0,"%d : Syntax Error", line_num-1);
	
  }
  pos = -pos -2;

  // Set problem to first instance of token in buf
  char * problem = strstr(buffer, token);
  char * token_orig = checked_malloc(sizeof(char *));
  strcpy(token_orig, token);
  // If we don't find anything (there are supposed to be newlines in the middle of the token
  char* mod_buf = checked_malloc(sizeof(char*));
  char error_char;
  while(problem == NULL && strlen(token) > 0)
  {
    // Keep deleting last char of the token until we find a match
    token[strlen(token)-1] = '\0';
    problem = strstr(buffer, token);
  }

// While we have not found the correct instance of token in buffer
do
{
  // If it is the wrong instance of token or a substring of token, 
  // we need to continue with the next match
  // To check if it is the wrong instance, 
  // compare token with a buffer-token stripped of newlines and see if it matches


  // printf("%d\n", strlen(token));

  error_char = token_orig[pos];
  int mod_pos = 0; // The position we are writing to
  int problem_pos = 0; // The position we are reading from
  int temp_pos = pos; // Temp created to not mess with pos
  //printf("%d\n", temp_pos);
  //printf("%c\n", problem[0]);
  while(problem[0] != '\n' && problem != start)
  {
    //printf("%c\t%c\t",problem[0], start[0]);
    //printf("%s\n", "inside");
    problem--;
  }
  if(problem[0]=='\n')
     problem++;
  for(problem_pos=0; problem_pos< temp_pos+1; problem_pos++)
  {
    //printf("%s", "In for loop.");
    if(problem[problem_pos] != '\n')
    {
      mod_buf[mod_pos] = problem[problem_pos];
      mod_pos++;
    }
    else
      temp_pos++;
  }
  mod_buf[mod_pos] = '\0';
  //printf("mod_buf: %s\n", mod_buf);
  //printf("pos: %d\n", pos);
  //printf("char: %c\n%c\n", mod_buf[pos], error_char);
  if(mod_buf[pos] == error_char)
  {
	//printf("%s\n", "Correct Instance");
	break;
  }
  //printf("%s\n", "Wrong Instance");
  while(problem[0] != '\n')
    problem++;
  problem++;
  problem = strstr(problem, token); 
} 
while(mod_buf[pos] != error_char);

  /*char* line = strtok(buffer, "\n"); 

  while(strstr(token, line) == NULL)
  {
  	printf("%s\n", line);
        line = strtok(NULL, "\n"); 
	if(*line == '\0') 
 	line_num++;
  }
  printf("%d\n", pos);
  while((unsigned) pos >= strlen(line))
  {
	printf("%d\n",strlen(line));
	pos -= strlen(line);
	line_num++;
	line = strtok(NULL, "\n");
	
  }
  printf("%d\n",strlen(line));
  printf("%d\n", pos); 
  */
   // We are at the at beginning of buffer counting newlines before the first instance of token in buf.
  while(buffer != problem)
  {
	if(*buffer == '\n')
	   line_num++;
	buffer++;
  }
  int j;
  // Up until we get to the position where we found the error, we check for newlines to add to the count
  for(j=0; j<pos; j++)
  {
    if(buffer[j] == '\n')
    {
        line_num++;
	pos++;
    }
  }
  error(1,0,"%d : Syntax Error", line_num);

}

// Checks to see if token is a complete command, return false if next line is needed
bool analyze_token(char* token)
{
	bool non_special = true;
	int depth = 0;
	int index = 0;
	char ch = token[index];
	while(ch != '\0')
	{
		if(ch == '(')
			depth++;
		else if(ch == ')')
			depth--;
		else if(ch == '|' || ch == '&' || ch == ';')
			non_special = false;
		else if(ch == ' ')
		{}  
		else
			non_special = true;
		index++;
		ch = token[index];
	}
	
	if(depth <= 0)
	{	
	//	printf("%s\n", token);
    		return non_special;
	}	
	else
	    return false;
}

command_stream_t
make_command_stream (int (*get_next_byte) (void *),
		     void *get_next_byte_argument)
{
  size_t size = 1000;
  char *buf = (char*) checked_malloc(size); 
  size_t i = 0;
  char c = get_next_byte(get_next_byte_argument);
  while(c != EOF)
  { 
     if(i >= size)
	checked_grow_alloc(buf, &size);
     buf[i]=c;
     c = get_next_byte(get_next_byte_argument);
     i += sizeof(char);
  }
  
  buf[i] = '\0';
  char* err_buf = checked_malloc(size);
  strcpy(err_buf, buf);
  //printf("%s\n", buf);
  // char delimiters  = 'a';
  char* token = strtok(buf, "\n"); 
  size = 1000;
  i = 0;
  // printf("here\n");
  command_stream_t result = checked_malloc(sizeof(command_stream_t));
  result->my_commands = checked_malloc(size);
  result->size = 0;
  result->next_pos = 0;

  while(token != NULL)
  {
    
     //printf("%s\n", token);
     if(analyze_token(token))
     {	
        //printf("%s\n", token);
  	result->my_commands[result->size] = process_buffer(token);
	//printf("Finished: %s\n", token);
	if(result->my_commands[result->size] == NULL)
	{}
	else if(result->my_commands[result->size]->status <= -2)
	{
	   // printf("BUF: %s\n", err_buf);
	   output_error(err_buf, token,
			result->my_commands[result->size]->status);
	}
	else
	{
	// print_command(result->my_commands[command_count]);
           result->size++;
        }
        token = strtok(NULL, "\n"); 
     //   printf("%s\n", token);
     }
     else
     {
       // printf("%s\n", token);
	char *temp = strtok(NULL, "\n");
	//printf("NEXT TOKEN: %s\n", temp);
	if(temp!=NULL)
	{
		strcat(token,temp);
        //	printf("%s\n", token);
	}
	else
	   output_error(err_buf, token,0);
     }
  }


/*ONE COMMAND TESTING PURPOSES ONLY
  command_t e1 =  process_buffer(set_of_tokens[0]);
  if(e1==NULL)
  {
	printf("NULL command, comment");
	return result;
  }
  // printf("Word 1: %s %d\n", e1->u.word[2], strlen(e1->u.word[0]));
  print_command(e1);
  // while(e1->u.word[index] != NULL)
  result->my_commands[0] = e1;
*/
  return result;

}

command_t process_buffer(char* buf)
{
  size_t i=0;
  //int line = 0;
  int depth = 1;
  bool input_flag = false;  // if < sign found, flag is true until input name is 			      parsed
  bool output_flag = false; // if > sign found, flag is true until output name  			       is parsed
  bool subshell_flag = false;
  char* subset = checked_malloc(1000);
  command_t entry = checked_malloc(sizeof(struct command));
  entry->status = 0;
  command_t complex_entry = checked_malloc(sizeof(struct command));
 // command_t subshell_entry = checked_malloc(sizeof(struct command));
//  if(entry != NULL)
  //	return entry;
  int word_count = 0;

  while(buf[i]!='\0')
  {
  //  printf("%d\n", i);
  //  printf("%c %d\n",buf[i], i);
    switch(buf[i])
    {
	case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g':
	case 'h': case 'i': case 'j': case 'k': case 'l': case 'm': case 'n':
	case 'o': case 'p': case 'q': case 'r': case 's': 
	case 'u': case 'v': case 'w': case 'x': case 'y': case 'z': 
	case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G': 
	case 'H': case 'I': case 'J': case 'K': case 'L': case 'M': case 'N':
	case 'O': case 'P': case 'Q': case 'R': case 'S': case 'T': case 'U':
	case 'V': case 'W': case 'X': case 'Y': case 'Z':
	case '0': case '1': case '2': case '3': case '4': case '5': case '6':
	case '7': case '8': case '9': case '!': case '%': case '+': case '_':
	case ',': case '-': case '.': case '/': case ':': case '@': case '^':
	case 't':
	{
		if(subshell_flag)
		{
			i++;
			break;
		}

		// entry = checked_malloc(sizeof(struct command));
		if(word_count==0)
		{
			entry->type = SIMPLE_COMMAND;
			entry->status = -1;
			entry->input = NULL;
			entry->output = NULL;
			entry->u.word = checked_malloc(1000);
		}
		char ch[2];
		ch[0] = buf[i];
		ch[1] = '\0';
		//printf("\t Character to be added: %s\n", ch);
		strcat(subset, ch);
		i++;
		break;
		
	}
	// Case dealing with subshells and white space at beginning of next line not implemented yet
        case ' ':
	case '\t':	
	{
		if(subshell_flag)
		{
			i++;
			break;
		}
		end_word(&input_flag, &output_flag, entry, subset, &word_count);
		i++;
		break;
	}
	case ';': 
	{
		if(subshell_flag)
		{
			i++;
			break;
		}
		end_word(&input_flag, &output_flag, entry, subset, &word_count);
		complex_entry->type = SEQUENCE_COMMAND;
		complex_entry->status = -1;
		complex_entry->input = NULL;
		complex_entry->output = NULL;
		complex_entry->u.command[0] = entry; 
		if(complex_entry->u.command[0]->status != -1)
		{
			complex_entry->status = -2 - i;
			return complex_entry;
		}
		complex_entry->u.command[1] = process_buffer(&buf[i+1]); 
		if(complex_entry->u.command[1] == NULL || 
                   complex_entry->u.command[1]->status <= -2)
			complex_entry->status = 
				complex_entry->u.command[1]->status - i - 1;
		return complex_entry;
	}
	case '|':
	{ 
	//ADD BOUNDS CHECKING	
		if(subshell_flag)
		{
			i++;
			break;
		}

		end_word(&input_flag, &output_flag, entry, subset, &word_count);
		if(buf[i+1] == '|')
		{
 		complex_entry->type = OR_COMMAND;
		complex_entry->status = -1;
		complex_entry->input = NULL;
		complex_entry->output = NULL;
		complex_entry->u.command[0] = entry; 
		if(complex_entry->u.command[0]->status != -1)
		{
			complex_entry->status = -2 - i;
			return complex_entry;
		}
		complex_entry->u.command[1] = process_buffer(&buf[i+2]); 
		if(complex_entry->u.command[1] == NULL || 
        complex_entry->u.command[1]->status <= -2)
			complex_entry->status = complex_entry->u.command[1]->status - i - 1;
		/*if(complex_entry->u.command[1] != NULL &&
		   complex_entry->u.command[1]->status <= -2)
			complex_entry->status =
				complex_entry->u.command[1]->status - i - 1 - 1 */
		  return complex_entry;
		}
		else if(buf[i+1] == '\0')
		{
			
		}
		else
		{
   		complex_entry->type = PIPE_COMMAND;
  		complex_entry->status = -1;
  		complex_entry->input = NULL;
  		complex_entry->output = NULL;
  		complex_entry->u.command[0] = entry; 
  		if(complex_entry->u.command[0]->status != -1)
  		{
  			complex_entry->status = -2 - i;
  			return complex_entry;
  		}
  		complex_entry->u.command[1] = process_buffer(&buf[i+1]); 
  		if(complex_entry->u.command[1] == NULL || 
                     complex_entry->u.command[1]->status <= -2)
  			complex_entry->status = 
  				complex_entry->u.command[1]->status - i - 1;
  		/*if(complex_entry->u.command[1] != NULL &&
  		   complex_entry->u.command[1]->status <= -2)
  			complex_entry->status = 
  				complex_entry->u.command[1]->status - i - 1;*/
  		return complex_entry;
		}
		break;
	}
	case '&': 
	{
		if(subshell_flag)
		{
			i++;
			break;
		}
		if(buf[i+1] != '&')// && Case
		{
			entry->status = -2 - i;
			//printf("here");
			return entry;
		}//	error(1,0,"%d : Syntax Error &&", line);
		
		end_word(&input_flag, &output_flag, entry, subset, &word_count);
		complex_entry->type = AND_COMMAND;
		complex_entry->status = -1;
		complex_entry->input = NULL;
		complex_entry->output = NULL;
		complex_entry->u.command[0] = entry; 
		if(complex_entry->u.command[0]->status != -1)
		{
			complex_entry->status = -2 - i;
			return complex_entry;
		}
		complex_entry->u.command[1] = process_buffer(&buf[i+2]); 
		if(complex_entry->u.command[1] != NULL &&
		    complex_entry->u.command[1]->status <= -2)
		{		
		complex_entry->status = 
			complex_entry->u.command[1]->status - i - 1 - 1;
	//subtract previous offset with current position, 1 for recursion, 1 for 	  && being 2 characters
		}
		return complex_entry;	
	}
	case '(': 
	{
		if(subshell_flag)
		{
			depth++;
			i++;
		}
		else
		{
	//	  entry = checked_malloc(sizeof(struct command));
		  entry->type = SUBSHELL_COMMAND;
		  entry->status = -1; // Unknown  
		  entry->input = NULL;
		  entry->output = NULL;
		  entry->u.subshell_command = process_buffer(&buf[i+1]);
		  if(entry->u.subshell_command != NULL &&
		     entry->u.subshell_command->status <= -2)
		  {
			entry->status = entry->u.subshell_command->status-i-1;
			return entry;
		  }//	checked_malloc(sizeof(struct command));
		  else if(entry->u.subshell_command->status != -1)
		  {
			entry->status = -2-i-1;
			return entry;
		  }
		  i++;
		  subshell_flag = true;
		}
		break;
	}
	case ')': 
	{
		//if(!subshell_flag) 
		  // error(1,0,"%d : Syntax Error )", line);
		if(subshell_flag)
		{
			depth--;
			i++;
			if(depth == 0)
			{  
			   subshell_flag = false;
			}
			break;
		}
		else
		{
			end_word(&input_flag, &output_flag, entry, 
				subset, &word_count);
		  	if(entry->status != -1)
			{
				entry->status = -2-i;
			}
			return entry;
		}
	}
	case '<':
	{
		if(subshell_flag)
		{
			i++;
			break;
		}
		if(input_flag) 
		{
		   entry->status = -2-i;
		   return entry;
		}   
		   //error(1,0,"%d : Syntax Error <", line);
 			
		end_word(&input_flag, &output_flag, entry, subset, &word_count);
		input_flag = true;
		i++;	
		break;
	}
	case '>':
	{
		if(subshell_flag)
		{
			i++;
			break;
		}
		if(output_flag)
		{
		   entry->status = -2-i;
		   return entry;
		}   
 		
		end_word(&input_flag, &output_flag, entry, subset, &word_count);
		output_flag = true;
		i++;	
		break;
	}
	case '#': //Set up check in calling function to ignore comments in the 
		  //command stream
	{
		return NULL;
	}
	default:
	{	
		entry->status = -2-i;
		return entry;
	}	//error(1,0,"%d : Syntax Error, Invalid Char %c", line, buf[i]);
	
    }
  }  
  
//  printf("Word 1: %s\n", entry->u.word[0]);
 // entry->u.word[word_count] = subset; 
  end_word(&input_flag, &output_flag, entry, subset, &word_count);
  if((input_flag && entry->input == NULL) || 
	(output_flag && entry->output == NULL))
	entry->status = -2 - i + 1;
  return entry;

}

void end_word(bool* input_flag, bool* output_flag, command_t entry, 
	      char* subset, int* word_count) 
{
		//printf("%d\n", strlen(subset));
		if (strlen(subset)==0)
		{
		 //  printf("breaking");
		   return;
		}
		else if(*input_flag)
		{
		   entry->input = checked_malloc(sizeof(subset));
		   strcpy(entry->input, subset);
		   *input_flag = false;
		}
		else if(*output_flag)
		{
		   entry->output = checked_malloc(sizeof(subset));
		   strcpy(entry->output, subset);
		   *output_flag = false;
		}
		else
		{
		   entry -> u.word[*word_count] = 
			checked_malloc(sizeof(subset));
		   strcpy(entry -> u.word[*word_count], subset);
		   (*word_count)++;
		}
		//printf("word 0: %s\n", entry->u.word[0]);
		//printf("word 1: %s\n", entry->u.word[1]);
		//printf("word 2: %s\n", entry->u.word[2]);
		subset[0] = '\0';

	//	printf("subset: %s\n", subset);

}
 // printf("%s\n", buf);

  //char delimiters  = 'a';

//  struct command_stream* cs = checked_malloc(sizeof(command_stream_t));
//  (*c).func = get_next_byte;
//  (*c).v = get_next_byte_argument;
//  return cs;

  //error (1, 0, "command reading not yet implemented");
  //return 0;

command_t read_command_stream (command_stream_t s)
{
  if(s->next_pos == s->size)
    return NULL;
  else if(s->my_commands[s->next_pos]->type != SIMPLE_COMMAND)
    reorder_commands(s);
  command_t temp = s->my_commands[s->next_pos];
  s->next_pos++;
  return temp;
}

void reorder_commands(command_stream_t s)
{
  // Reorder tree
  command_t temp = s->my_commands[s->next_pos];
  // printf("Before: \n");
  // print_command(temp);

  // while
  while(temp->u.command[1] != NULL && temp->u.command[1]->type != SIMPLE_COMMAND)
  {
    command_t new_right = temp->u.command[1]->u.command[0]; 
    command_t new_left = temp->u.command[0];
    command_t new_entry = temp;
    temp = temp->u.command[1];
    // new_entry->u.command[0] = new_left;
    new_entry->u.command[1] = new_right;
    temp->u.command[0] = new_entry;
    // temp->u.command[1] stays the same
  }
  s->my_commands[s->next_pos] = temp;
  // printf("After: \n");
}
