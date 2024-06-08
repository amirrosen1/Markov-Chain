snake: linked_list.o markov_chain.o snakes_and_ladders.o
	gcc linked_list.o markov_chain.o snakes_and_ladders.o -o snakes_and_ladders

tweets: linked_list.o markov_chain.o tweets_generator.o
	gcc linked_list.o markov_chain.o tweets_generator.o -o tweets_generator

linked_list.o: linked_list.c linked_list.h
	gcc -c linked_list.c

markov_chain.o: markov_chain.c markov_chain.h linked_list.h
	gcc -c markov_chain.c

tweets_generator.o: tweets_generator.c markov_chain.h linked_list.h
	gcc -c tweets_generator.c

snakes_and_ladders.o : snakes_and_ladders.c markov_chain.h linked_list.h
	gcc -c snakes_and_ladders.c

clean:
	rm *.o