#include "markov_chain.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#define INVALID_NUMBER_OF_ARGUMENTS "Usage: The correct arguments are: int"\
" seed, number of tweets, file's path, and it is also possible to bring an"\
" amount of words to read from the file.\n"
#define SEED argv[1]
#define AMOUNT_OF_TWEETS argv[2]
#define IN_FILE_ARG argv[3]
#define WORD_TO_READ_FROM_FILE argv[4]
#define VALUE_FOR_THE_LOOP 1
#define MIN_ARG 4
#define MAX_ARG 5
#define MAX_WORD_TO_TWEET 20
#define MAX_LEN_TWEET 1001

static bool string_is_last (const void *src);

static void string_print (const void *a_data)
{
  const char *target = (const char *) a_data;
  if (string_is_last (a_data))
  {
    fprintf (stdout, "%s", (const char *) target);
  }
  else
  {
    fprintf (stdout, "%s ", (const char *) target);
  }
}

static int string_cmp (const void *a_data, const void *b_data)
{
  const char *first = (const char *) a_data;
  const char *second = (const char *) b_data;
  return strcmp (first, second);
}

static void string_free (const void *a_data)
{
  const char *target = (const char *) a_data;
  free ((char *) target);
}

static void *string_cpy (const void *src)
{
  const char *target = (const char *) src;
  void *dest = calloc (1, strlen (target) + 1);
  if (!dest)
  {
    return NULL;
  }
  char *dest_target = (char *) dest;
  strcpy (dest_target, target);
  return dest;
}

static bool string_is_last (const void *src)
{
  const char *target = (const char *) src;
  if (target[(strlen (target)) - 1] != '.')
  {
    return false;
  }
  return true;
}

static int fill_database (FILE *fp, int words_to_read, MarkovChain
*markov_chain)
{
  char line[MAX_LEN_TWEET] = {0};
  int counter_word = 0;
  while ((fgets (line, MAX_LEN_TWEET, fp)))
  {
    if ((counter_word >= words_to_read) && (words_to_read != 0))
    {
      break;
    }
    char *first_word = strtok (line, " " "\r" "\n");
    if (!first_word)
    {
      continue;
    }
    Node *first_node = add_to_database (markov_chain, first_word);
    counter_word += 1;
    if (!first_node)
    {
      return EXIT_FAILURE;
    }
    while (VALUE_FOR_THE_LOOP)
    {
      if ((counter_word >= words_to_read) && (words_to_read != 0))
      {
        break;
      }
      char *change_word = strtok (NULL, " " "\n" "\r");
      if (!change_word)
      {
        break;
      }
      Node *change_node = add_to_database (markov_chain, change_word);
      counter_word += 1;
      if (!change_node)
      {
        return EXIT_FAILURE;
      }
      if (first_word[(strlen (first_word)) - 1] != '.')
      {
        bool combine_words = add_node_to_counter_list (
            first_node->data, change_node->data,
            markov_chain);
        if (!combine_words)
        {
          return EXIT_FAILURE;
        }
      }
      first_word = change_word;
      first_node = change_node;
    }
  }
  return EXIT_SUCCESS;
}

/**
 * The function checks the validity of the arguments.
 * @param argc: Amount of arguments.
 * @param argv: The arguments.
 * @return : If everything is OK, the function returns EXIT_SUCCESS, otherwise
 * it returns a message accordingly.
 */
static int validate_tweets (int argc, char *argv[])
{
  if (!IN_FILE_ARG)
  {
    return EXIT_FAILURE;
  }
  if (argc != MIN_ARG && argc != MAX_ARG)
  {
    fprintf (stdout, INVALID_NUMBER_OF_ARGUMENTS);
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

/**
 * The function generates the sentences (tweets).
 * @param markov_print: The markov chain.
 * @param max_tweet: Maximum amount of tweets.
 */
static void generate_tweets (MarkovChain *markov_print, long max_tweet)
{
  for (int i = 1; i <= max_tweet; i++)
  {
    fprintf (stdout, "Tweet %d: ", i);
    generate_random_sequence (markov_print, NULL,
                              MAX_WORD_TO_TWEET);
    fprintf (stdout, "\n");
  }
}

/**
 * The function generates new markov chain.
 * @param print : Print function.
 * @param comp : Comparison function.
 * @param free_func : Free function.
 * @param copy_func : Copy function.
 * @param last_func : Is last function.
 * @return If it succeeded, pointer to the new markov chain, otherwise NULL.
 */
static MarkovChain *
create_new_markov_chain_tweets (print_func_t print, comp_func_c comp,
                                free_data_f free_func, copy_func_cpy copy_func,
                                is_last_lst last_func)
{
  MarkovChain *new_markov_chain = calloc (1, sizeof (MarkovChain));
  if (new_markov_chain == NULL)
  {
    return NULL;
  }
  LinkedList *current_database = NULL;
  current_database = calloc (1, sizeof (LinkedList));
  if (current_database == NULL)
  {
    free (new_markov_chain);
    new_markov_chain = NULL;
    return NULL;
  }
  new_markov_chain->database = current_database;
  new_markov_chain->print_func = print;
  new_markov_chain->comp_func = comp;
  new_markov_chain->free_data = free_func;
  new_markov_chain->copy_func = copy_func;
  new_markov_chain->is_last = last_func;
  return new_markov_chain;
}

int main (int argc, char *argv[])
{
  int res = validate_tweets (argc, argv);
  if (res == EXIT_FAILURE)
  {
    return EXIT_FAILURE;
  }
  FILE *in_file = fopen (IN_FILE_ARG, "r");
  if (in_file == NULL)
  {
    fclose (in_file);
    return EXIT_FAILURE;
  }
  long seed_tweet = strtol (SEED, NULL, 10);
  srand (seed_tweet);
  long tweet_count = strtol (AMOUNT_OF_TWEETS, NULL, 10);
  long word_to_read = 0;
  if (argc == MAX_ARG)
  {
    word_to_read = strtol (WORD_TO_READ_FROM_FILE, NULL, 10);
  }
  MarkovChain *markov_chain = create_new_markov_chain_tweets (
      string_print,
      string_cmp,
      string_free,
      string_cpy,
      string_is_last);
  if (!markov_chain)
  {
    free_markov_chain (&markov_chain);
    fprintf (stdout, ALLOCATION_ERROR_MASSAGE);
    return EXIT_FAILURE;
  }
  int check = fill_database (in_file, (int) word_to_read,
                             markov_chain);
  fclose (in_file);
  if (check)
  {
    free_markov_chain (&markov_chain);
    fprintf (stdout, ALLOCATION_ERROR_MASSAGE);
    return EXIT_FAILURE;
  }
  generate_tweets (markov_chain, tweet_count);
  free_markov_chain (&markov_chain);
  return EXIT_SUCCESS;
}