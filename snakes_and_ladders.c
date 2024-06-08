#include <string.h> // For strlen(), strcmp(), strcpy()
#include "markov_chain.h"

#define MAX(X, Y) (((X) < (Y)) ? (Y) : (X))
#define EMPTY -1
#define BOARD_SIZE 100
#define MAX_GENERATION_LENGTH 60
#define DICE_MAX 6
#define NUM_OF_TRANSITIONS 20
#define MAX_ARG 3
#define INVALID_NUMBER_OF_ARGUMENTS "Usage: The correct arguments are: int"\
" seed, number of sentence to generate.\n"
#define SEED_BOARD argv[1]
#define AMOUNT_OF_SENTENCE argv[2]

/**
 * represents the transitions by ladders and snakes in the game
 * each tuple (x,y) represents a ladder from x to if x<y or a snake otherwise
 */
const int transitions[][2] = {{13, 4},
                              {85, 17},
                              {95, 67},
                              {97, 58},
                              {66, 89},
                              {87, 31},
                              {57, 83},
                              {91, 25},
                              {28, 50},
                              {35, 11},
                              {8,  30},
                              {41, 62},
                              {81, 43},
                              {69, 32},
                              {20, 39},
                              {33, 70},
                              {79, 99},
                              {23, 76},
                              {15, 47},
                              {61, 14}};

/**
 * struct represents a Cell in the game board
 */
typedef struct Cell
{
    int number; // Cell number 1-100
    int ladder_to;  // ladder_to represents the jump of the ladder in case
    // there is one from this square
    int snake_to;  // snake_to represents the jump of the snake in case there
    // is one from this square
    //both ladder_to and snake_to should be -1 if the Cell doesn't have them
} Cell;

static void cell_print (const void *a_data)
{
  Cell *target = (Cell *) a_data;
  if (target->ladder_to != EMPTY)
  {
    fprintf (stdout, "[%d]-ladder to %d -> ", target->number,
             target->ladder_to);
  }
  if (target->snake_to != EMPTY)
  {
    fprintf (stdout, "[%d]-snake to %d -> ", target->number,
             target->snake_to);
  }
  if (target->number == BOARD_SIZE)
  {
    fprintf (stdout, "[%d]", target->number);
  }
  else if (target->ladder_to == EMPTY && target->snake_to == EMPTY)
  {
    fprintf (stdout, "[%d] -> ", target->number);
  }
}

static int cell_cmp (const void *a_data, const void *b_data)
{
  Cell *target_a = (Cell *) a_data;
  Cell *target_b = (Cell *) b_data;
  return target_a->number - target_b->number;
}

static void cell_free (const void *a_data)
{
  Cell *target = (Cell *) a_data;
  free (target);
}

static void *cell_cpy (const void *src)
{
  Cell *target = (Cell *) src;
  void *dest = calloc (1, sizeof (Cell));
  if (!dest)
  {
    return NULL;
  }
  Cell *dest_target = (Cell *) dest;
  dest_target->number = target->number;
  dest_target->ladder_to = target->ladder_to;
  dest_target->snake_to = target->snake_to;
  return dest;
}

static bool cell_is_last (const void *src)
{
  Cell *target = (Cell *) src;
  if (target->number == BOARD_SIZE)
  {
    return true;
  }
  return false;
}

/** Error handler **/
static int handle_error (char *error_msg, MarkovChain **database)
{
  printf ("%s", error_msg);
  if (database != NULL)
  {
    free_markov_chain (database);
  }
  return EXIT_FAILURE;
}

static int create_board (Cell *cells[BOARD_SIZE])
{
  for (int i = 0; i < BOARD_SIZE; i++)
  {
    cells[i] = malloc (sizeof (Cell));
    if (cells[i] == NULL)
    {
      for (int j = 0; j < i; j++)
      {
        free (cells[j]);
      }
      handle_error (ALLOCATION_ERROR_MASSAGE, NULL);
      return EXIT_FAILURE;
    }
    *(cells[i]) = (Cell) {i + 1, EMPTY, EMPTY};
  }

  for (int i = 0; i < NUM_OF_TRANSITIONS; i++)
  {
    int from = transitions[i][0];
    int to = transitions[i][1];
    if (from < to)
    {
      cells[from - 1]->ladder_to = to;
    }
    else
    {
      cells[from - 1]->snake_to = to;
    }
  }
  return EXIT_SUCCESS;
}

/**
 * fills database
 * @param markov_chain
 * @return EXIT_SUCCESS or EXIT_FAILURE
 */
static int fill_database (MarkovChain *markov_chain)
{
  Cell *cells[BOARD_SIZE];
  if (create_board (cells) == EXIT_FAILURE)
  {
    return EXIT_FAILURE;
  }
  MarkovNode *from_node = NULL, *to_node = NULL;
  size_t index_to;
  for (size_t i = 0; i < BOARD_SIZE; i++)
  {
    add_to_database (markov_chain, cells[i]);
  }

  for (size_t i = 0; i < BOARD_SIZE; i++)
  {
    from_node = get_node_from_database (markov_chain, cells[i])->data;

    if (cells[i]->snake_to != EMPTY || cells[i]->ladder_to != EMPTY)
    {
      index_to = MAX(cells[i]->snake_to, cells[i]->ladder_to) - 1;
      to_node = get_node_from_database (markov_chain, cells[index_to])
          ->data;
      add_node_to_counter_list (from_node, to_node,
                                markov_chain);
    }
    else
    {
      for (int j = 1; j <= DICE_MAX; j++)
      {
        index_to = ((Cell *) (from_node->data))->number + j - 1;
        if (index_to >= BOARD_SIZE)
        {
          break;
        }
        to_node = get_node_from_database (markov_chain,
                                          cells[index_to])
            ->data;
        add_node_to_counter_list (from_node, to_node,
                                  markov_chain);
      }
    }
  }
  // free temp arr
  for (size_t i = 0; i < BOARD_SIZE; i++)
  {
    free (cells[i]);
  }
  return EXIT_SUCCESS;
}

/**
 * The function checks the validity of the arguments.
 * @param argc: Amount of arguments.
 * @return : If everything is OK, the function returns EXIT_SUCCESS, otherwise
 * it returns a message accordingly.
 */
static int validate_snake (int argc)
{
  if (argc != MAX_ARG)
  {
    fprintf (stdout, INVALID_NUMBER_OF_ARGUMENTS);
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

/**
 * The function generates the paths.
 * @param markov_print: The markov chain.
 * @param max_print: Maximum amount of paths.
 */
static void generate_sentence (MarkovChain *markov_print, long max_sentence)
{
  for (int i = 1; i <= max_sentence; i++)
  {
    fprintf (stdout, "Random Walk %d: ", i);
    generate_random_sequence (markov_print,
                              markov_print->database->first->data,
                              MAX_GENERATION_LENGTH);
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
create_new_markov_chain (print_func_t print, comp_func_c comp,
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

/**
 * @param argc num of arguments
 * @param argv 1) Seed
 *             2) Number of sentences to generate
 * @return EXIT_SUCCESS or EXIT_FAILURE
 */
int main (int argc, char *argv[])
{
  int res = validate_snake (argc);
  if (res == EXIT_FAILURE)
  {
    return EXIT_FAILURE;
  }
  long seed_snake = strtol (SEED_BOARD, NULL, 10);
  srand (seed_snake);
  long sentence_count = strtol (AMOUNT_OF_SENTENCE, NULL, 10);
  MarkovChain *markov_chain = create_new_markov_chain (cell_print,
                                                       cell_cmp,
                                                       cell_free,
                                                       cell_cpy,
                                                       cell_is_last);
  if (!markov_chain)
  {
    free_markov_chain (&markov_chain);
    fprintf (stdout, ALLOCATION_ERROR_MASSAGE);
    return EXIT_FAILURE;
  }
  int check = fill_database (markov_chain);
  if (check)
  {
    free_markov_chain (&markov_chain);
    fprintf (stdout, ALLOCATION_ERROR_MASSAGE);
    return EXIT_FAILURE;
  }
  generate_sentence (markov_chain, sentence_count);
  free_markov_chain (&markov_chain);
  return EXIT_SUCCESS;
}
