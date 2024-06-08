#include "markov_chain.h"
#include <string.h>
#define NOT_FOUND -555
#define FAILURE "false"
#define SUCCESS "true"


 void helper_print (MarkovChain *markov_chain, MarkovNode *new_one,
                   MarkovNode *change_node, int count_print, int max_length);
int help_search_node (MarkovNode *head, void *data, MarkovChain *markov_chain);
MarkovNode *
create_new_markov_node (void *data_ptr, NextNodeCounter *arr, int cur_sum,
                        int cur_count, MarkovChain *markov_chain);

/**
* Get random number between 0 and max_number [0, max_number).
* @param max_number maximal number to return (not including)
* @return Random number
*/
int get_random_number (int max_number)
{
  return rand () % max_number;
}

MarkovNode *get_first_random_node (MarkovChain *markov_chain)
{
  int len_of_lst = markov_chain->database->size;
  Node *change_head = NULL;
  int flag = 0;
  while (!flag)
  {
    int random_number_len = get_random_number (len_of_lst);
    change_head = markov_chain->database->first;
    for (int i = 0; i < random_number_len; ++i)
    {
      change_head = change_head->next;
    }
    if (!(markov_chain)->is_last (change_head->data->data))
    {
      flag = 1;
    }
  }
  return change_head->data;
}

MarkovNode *get_next_random_node (MarkovNode *state_struct_ptr)
{
  if (state_struct_ptr->counter_list == NULL)
  {
    return NULL;
  }
  int freq_sum = state_struct_ptr->sum_of_all_data;
  if (!freq_sum)
  {
    return NULL;
  }
  int rand_num = get_random_number (freq_sum);
  for (int i = 0; i < state_struct_ptr->count_data; i++)
  {
    rand_num -= state_struct_ptr->counter_list[i].frequency;
    if (rand_num < 0)
    {
      return state_struct_ptr->counter_list[i].markov_node;
    }
  }
  return NULL;
}

/**
 * A function that "helps" to print each data.
 * @param markov_chain : The markov chain.
 * @param new_one : Current node to print.
 * @param change_node : Change node to print.
 * @param count_print : A variable that counts all the data printed so far.
 * @param max_length : Max length of data to print.
 */
void helper_print (MarkovChain *markov_chain, MarkovNode *new_one,
                   MarkovNode *change_node, int count_print, int max_length)
{
  markov_chain->print_func (new_one->data);
  count_print += 1;
  while ((count_print < max_length))
  {
    change_node = get_next_random_node (new_one);
    if (markov_chain->is_last (change_node->data))
    {
      markov_chain->print_func (change_node->data);
      return;
    }
    markov_chain->print_func (change_node->data);
    count_print += 1;
    new_one = change_node;
  }
}

void generate_random_sequence (MarkovChain *markov_chain, MarkovNode *
first_node, int max_length)
{
  MarkovNode *new_one;
  if (first_node == NULL)
  {
    new_one = get_first_random_node (markov_chain);
  }
  else
  {
    new_one = first_node;
  }
  int count_print = 0;
  MarkovNode *change_node = NULL;
  helper_print (markov_chain, new_one, change_node, count_print, max_length);
}

void free_markov_chain (MarkovChain **ptr_chain)
{
  Node *temp;
  Node *head = (*ptr_chain)->database->first;
  while (head != NULL)
  {
    temp = head;
    head = head->next;
    (*ptr_chain)->free_data (temp->data->data);
    temp->data->data = NULL;
    free (temp->data->counter_list);
    temp->data->counter_list = NULL;
    free (temp->data);
    temp->data = NULL;
    free (temp);
    temp = NULL;
  }
  free ((*ptr_chain)->database);
  (*ptr_chain)->database = NULL;
  free (*ptr_chain);
  *ptr_chain = NULL;
}

/**
 * The function checks if a data appears in the array of counter list.
 * @param head : Pointer to a node.
 * @param data : A void pointer of data.
 * @param markov_chain : The markov chain.
 * @return : If the data is found, the function returns its position in the
 * array, otherwise it returns that it is not found.
 */
int help_search_node (MarkovNode *head, void *data, MarkovChain *markov_chain)
{
  for (int i = 0; i < head->count_data; i++)
  {
    void *change_data = head->counter_list[i].markov_node->data;
    if (markov_chain->comp_func (change_data, data) == 0)
    {
      return i; // return the index of the data
    }
  }
  return NOT_FOUND;
}

bool add_node_to_counter_list (MarkovNode *first_node, MarkovNode *second_node,
                               MarkovChain *markov_chain)
{
  if ((first_node->data == NULL) || (second_node->data == NULL))
  {
    return FAILURE;
  }
  int check = help_search_node (first_node, second_node->data,
                                markov_chain);
  if (check != NOT_FOUND)
  {
    first_node->sum_of_all_data += 1;
    first_node->counter_list[check].frequency += 1;
  }
  else
  {
    NextNodeCounter *change = realloc (first_node->counter_list,
                                       (first_node->count_data + 1)
                                       * (sizeof (NextNodeCounter)));
    if (!change)
    {
      free (first_node->counter_list);
      first_node->counter_list = NULL;
      return FAILURE;
    }
    first_node->sum_of_all_data += 1;
    first_node->count_data += 1;
    first_node->counter_list = change;
    first_node->counter_list[first_node->count_data - 1].markov_node =
        second_node;
    first_node->counter_list[first_node->count_data - 1].frequency = 1;
  }
  return SUCCESS;
}

Node *get_node_from_database (MarkovChain *markov_chain, void *data_ptr)
{
  if ((markov_chain == NULL) || (data_ptr == NULL))
  {
    return NULL;
  }
  Node *current = NULL;
  current = markov_chain->database->first;
  while (current != NULL)
  {
    if (markov_chain->comp_func (current->data->data, data_ptr) == 0)
    {
      return current;
    }
    else
    {
      current = current->next;
    }
  }
  return NULL;
}

/**
 * The function creates new markov node.
 * @param data_ptr: A void pointer of data.
 * @param arr: Array of next node counter.
 * @param cur_sum: The total sum of all repetitions (repetitions of data).
 * @param cur_count: The amount of all the data.
 * @param markov_chain : The markov chain.
 * @return If the function succeeds, pointer to a new markov node, otherwise
 * NULL.
 */
MarkovNode *
create_new_markov_node (void *data_ptr, NextNodeCounter *arr, int cur_sum,
                        int cur_count, MarkovChain *markov_chain)
{
  MarkovNode *new_one = calloc (1, sizeof (MarkovNode));
  if (!new_one)
  {
    return NULL;
  }
  if (!(new_one->data = markov_chain->copy_func (data_ptr)))
  {
    free (new_one);
    new_one = NULL;
    return NULL;
  }
  new_one->counter_list = arr;
  new_one->count_data = cur_count;
  new_one->sum_of_all_data = cur_sum;
  return new_one;
}

Node *add_to_database (MarkovChain *markov_chain, void *data_ptr)
{
  if (markov_chain->database == NULL)
  {
    return NULL;
  }
  Node *check_in = get_node_from_database (markov_chain, data_ptr);
  if (!check_in)
  {
    MarkovNode *new_node = create_new_markov_node (data_ptr, NULL,
                                                   0, 0,
                                                   markov_chain);
    if (!new_node)
    {
      return NULL;
    }
    int add_to_database_arg = add (markov_chain->database,
                                   new_node);
    if (add_to_database_arg)
    {
      markov_chain->free_data (new_node->data);
      new_node->data = NULL;
      free (new_node);
      new_node = NULL;
      return NULL;
    }
    return markov_chain->database->last;
  }
  return check_in;
}