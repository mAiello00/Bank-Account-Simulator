/*
 * Marco Aiello
 * COMPSCI 3305
 * Assignment 5
 * Due: November 29, 2021
 * 251 047 101
 *
 * */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

// Create the mutex lock
pthread_mutex_t lock;

struct TransactionRequest
{
	// ID if the client doing the transaction
	int client;
	// Type of transaction the client is doing
	char requestType;
	// Account number of the account the client is dealing with
	int account;
	// Amount of money that is boing moved around
	int amount;
};

struct BankAccount
{
	int accNum;
	int balance;
};

struct BankAccount arrAccounts[20];
struct TransactionRequest arrTransactions[20];

void *BankTransaction(void *rqst)
{
	struct TransactionRequest *request  = (struct TransactionRequest*)rqst;

	pthread_mutex_lock(&lock);

	//printf("This is the client doing the transaction: %d\n", request->client);
	//printf("This is the type of request: %s\n", request->requestType);
	//printf("This is the account number: %d\n", request->account);
	//printf("This is the amount: %d\n", request->amount);
	//printf("\n\n");
	
	if(request->requestType == 'd')
	{
		arrAccounts[(request->account)-1].balance += request->amount;
	}
	else if(request->requestType == 'w')
	{
		int bal = arrAccounts[(request->account)-1].balance;

		if((bal - request->amount) < 0)
			printf("This transaction could not be completed. Not enough funds in account\n");
		else
		{
			arrAccounts[(request->account)-1].balance -= request->amount;
		}

		printf("Account %d Balance %d\n", arrAccounts[(request->account)-1].accNum, arrAccounts[(request->account)-1].balance);
	}

	pthread_mutex_unlock(&lock);
}

int main()
{
	// Open the file
	FILE *fp;
	char line[100];
	fp = fopen("assignment_6_input.txt","r");
	if(fp == NULL)
	{
		printf("Could not find the input file");
		return 0;
	}

	char wordFirstLetter;
	int numAccounts = 0;
	int numTransactions = 0;
	int currAccountIndex = 0;
	int currClient = 1;

	while(fgets(line, sizeof(line),fp))
	{
		// We will be getting the words on each line using strtok()
		//Gets the first word in the line
		char *word = strtok(line, " ");
		wordFirstLetter = word[0];

		// Checks if the first letter of the line is 'a'
		// If it is, then we know we are setting up the account
		if(wordFirstLetter == 'a')
		{
			numAccounts += 1;

			int aNum = word[1] - '0';
			//printf("This is the account number we just got: %d\n",aNum);

			word = strtok(NULL, " ");
			word = strtok(NULL, " ");
			int initialBalance = atoi(word);
			
			// Set up each bank account
			struct BankAccount account;
			account.accNum = aNum;
			account.balance = initialBalance;
			//printf("This is the initial balance we just got: %d\n");

			arrAccounts[currAccountIndex] = account;

			currAccountIndex++;
		}
		// Otherwise we are dealing with a client
		else
		{
			while(word != NULL)
			{
				if(word[0] == 'c')
				{
					// Move on to the next word if we see that we are
					// dealing with a client
					word = strtok(NULL, " ");
				}
				else
				{
					// Create a structure to represent the current request
					struct TransactionRequest transaction;

					// Append the client number of the client doing the transaction
					transaction.client = currClient;
					

					// Append the type of transaction and move to next word
					transaction.requestType = word[0];
					word = strtok(NULL, " ");
					

					// Append the account number that will be altered and move to next word
					transaction.account = word[1] - '0';
					word = strtok(NULL, " ");
					

					// Append the amount for the transaction and move to next word
					int transAmount = atoi(word);
					transaction.amount = transAmount;
					//printf("This is the amount in transAmount: %d\n", transAmount);
					//printf("This is the amount in the struct: %d\n", transaction.amount);
					word = strtok(NULL, " ");
					

					// Append this transaction to an array containing all transactions in the file
					arrTransactions[numTransactions] = transaction;
					numTransactions++;
				}
			}
			// Each line represents a client, so when we are done with the
			// line we move to the next client
			currClient++;
		}
	}
	
	for(int i = 0; i < numAccounts; i++)
	{
		struct BankAccount account = arrAccounts[i];
		printf("Account %d Balance %d\n", account.accNum, account.balance);
	}
	
	printf("\n\n");

	for(int i = 0; i < numTransactions; i++)
	{
		struct TransactionRequest transaction = arrTransactions[i];
		printf("This is the client number: %d\n", transaction.client);
		printf("This is the request type: %c\n", transaction.requestType);
		printf("This is the account to be accessed: %d\n", transaction.account);
		printf("This is the amount: %d\n", transaction.amount);
		printf("\n");
	}
	

	// Done reading the file
	fclose(fp);
	
	int err_thread;
	pthread_t threads[numTransactions];

	if(pthread_mutex_init(&lock, NULL) != 0)
	{
		printf("\n Mutex init failed\n");
		return 1;
	}

	for(int i = 0; i < numTransactions; i++)
	{
		struct TransactionRequest t = arrTransactions[i];
		err_thread = pthread_create(&threads[i], NULL, BankTransaction, &t);
		if(err_thread != 0)
		{
			printf("Error in creating thread\n");
		}
	}

	for(int i = 0; i < numTransactions; i++)
	{
		pthread_join(threads[i],NULL);
	}

	pthread_mutex_destroy(&lock);

	for(int i = 0; i < numAccounts; i++)
        {
                struct BankAccount account = arrAccounts[i];
                printf("Account %d Balance %d\n", account.accNum, account.balance);
        }
		
}

