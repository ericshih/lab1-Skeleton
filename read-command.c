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

struct command_stream
{
  command_t* my_commands;

  //int (*func) (void*); //function pointer
  //void* v;	       //void *
};

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
  //printf("%s\n", buf);
  // char delimiters  = 'a';
  char* token = strtok(buf, "\n"); 
  size = 1000;
  i = 0;
  // printf("here\n");
  char** set_of_tokens = checked_malloc(size);
 

  while(token != NULL)
  {
     if(i >= size)
	checked_grow_alloc(set_of_tokens, &size);
     
     set_of_tokens[i] = token;
  //   printf("%s\n", token);
     token = strtok(NULL, "\n"); 
     i++;

  }
  size_t j =0;
  printf("%d", i);

  command_stream_t result = checked_malloc(sizeof(command_stream_t));
  result->my_commands = checked_malloc(size);
  //int command_count=0;
  for(j =0; j < i; j++)
  {
  	printf("Token %d: %s\n",j, set_of_tokens[j]);
/*  	result->my_commands[command_count] = process_buffer(set_of_tokens[j]); 
	if(result->my_commands[command_count] != NULL)
	{
	   print_command(result->my_commands[command_count]);
	   command_count++;
        }
*/  }

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

  return result;
}

command_t process_buffer(char* buf)
{
  size_t i=0;
  int line = 0;
  int depth = 1;
  bool input_flag = false;  // if < sign found, flag is true until input name is 			      parsed
  bool output_flag = false; // if > sign found, flag is true until output name  			       is parsed
  bool subshell_flag = false;
  char* subset = checked_malloc(1000);
  command_t entry = checked_malloc(sizeof(struct command));
  command_t complex_entry = checked_malloc(sizeof(struct command));
 // command_t subshell_entry = checked_malloc(sizeof(struct command));
//  if(entry != NULL)
  //	return entry;
  int word_count = 0;

  while(buf[i]!='\0')
  {
    printf("%c %d\n",buf[i], i);
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
		printf("\t Character to be added: %s\n", ch);
		strcat(subset, ch);
		i++;
		break;
		
	}
        case ' ':
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
	//	printf("semicolon found"); 
		
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

	//	complex_entry->u.command[0] = 
	//		checked_malloc(sizeof(struct command));

	//	memcpy(complex_entry->u.command[0], entry,
	//		sizeof(struct command));

	//	complex_entry->u.command[1] = 
	//		checked_malloc(sizeof(struct command));

		complex_entry->u.command[1] = process_buffer(&buf[i+1]); 
	//	memcpy(complex_entry->u.command[1], process_buffer(&buf[i+1]), 
	//		sizeof(struct command));
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
		complex_entry->u.command[1] = process_buffer(&buf[i+2]); 
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
		complex_entry->u.command[1] = process_buffer(&buf[i+1]); 
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
			error(1,0,"%d : Syntax Error &&", line);
		
		end_word(&input_flag, &output_flag, entry, subset, &word_count);
		complex_entry->type = AND_COMMAND;
		complex_entry->status = -1;
		complex_entry->input = NULL;
		complex_entry->output = NULL;
		complex_entry->u.command[0] = entry; 
		complex_entry->u.command[1] = process_buffer(&buf[i+2]); 
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
	//	entry = checked_malloc(sizeof(struct command));
		entry->type = SUBSHELL_COMMAND;
		entry->status = -1; // Unknown  
		entry->input = NULL;
		entry->output = NULL;
		entry->u.subshell_command = process_buffer(&buf[i+1]);
		//	checked_malloc(sizeof(struct command));
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
		   error(1,0,"%d : Syntax Error <", line);
 			
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
		   error(1,0,"%d : Syntax Error >", line);
 		
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
	case '\t':	
	
		break;
	case '\n':
	{
		line++;
		return NULL;
		break;
	}
	default:
		error(1,0,"%d : Syntax Error, Invalid Char %c", line, buf[i]);
	
    }
  }  
  
//  printf("Word 1: %s\n", entry->u.word[0]);
 // entry->u.word[word_count] = subset; 
  end_word(&input_flag, &output_flag, entry, subset, &word_count);
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
		printf("word 0: %s\n", entry->u.word[0]);
		printf("word 1: %s\n", entry->u.word[1]);
		printf("word 2: %s\n", entry->u.word[2]);
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

command_t
read_command_stream (command_stream_t s)
{
  s=s;  
  
  /* FIXME: Replace this with your implementation too.  */
  error (1, 0, "command reading not yet implemented");
  return 0;
}
