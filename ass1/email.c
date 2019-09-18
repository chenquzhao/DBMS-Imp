#include "postgres.h"

#include "fmgr.h"
#include "libpq/pqformat.h"	

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

PG_MODULE_MAGIC;

typedef struct EmailAddr
{
	int32 length;
	char content[FLEXIBLE_ARRAY_MEMBER];
}	EmailAddr;

int is_valid_email(char *email);

/*
	Check if email is valid.
*/
int is_valid_email(char *email) {
	static char symbol[] = {'@', '.', '-'};
	int domain_word = 0; 
	int count = 0;			// number of @ 
	char i = *email;
	char *s;		
	int result = 1;

	/*  local + '@' + domain + '\0' */
	if(sizeof(email) > (2*256+1+1))
	{
		result = 0;
	}
	
	for(s = email; *s != '\0'; ++s) {

		/*  only one '@' allowed. */
		if(*s == '@') {
			count += 1;
			if(count > 1)
			 {
				 result = 0;
			 }
		}
		/* character should be specifed symbol or english letter or digit */
		if(!(strchr(symbol, *s)||isalpha(*s) || isdigit(*s)))
		{
			result = 0;
		}

		/* word should end with a letter or digit */
		if(!isalpha(i) && !isdigit(i) && (*s == '.' || *s == '@' || *(s+1) == '\0'))
		{
			result = 0;
		}

		/*  local part should start with a letter */
		if(s == email && !isalpha(*s))
		{
			result = 0;
		}

		/* middle part should start with a letter */
		if(!isalpha(*s) && (i == '.' || i == '@')) 
		{
			result = 0;
		}

		if(count == 1 && ( i == '@' || i == '.'))
		{
			domain_word += 1;
		}

		i = *s;	

	}

	/* Domain part has more than two word */
	if(domain_word < 2)
	{
		result = 0;
	}

	return result;	
}

/*
	Input function.
 */
PG_FUNCTION_INFO_V1(email_in);

Datum
email_in(PG_FUNCTION_ARGS)
{
	int i = 0;
	char *str = PG_GETARG_CSTRING(0);
	EmailAddr *result;

	while(str[i])
	{
		putchar(tolower(str[i]));
        i++;
	}

	if(!is_valid_email(str))
	{
		ereport(ERROR,
				(errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
				 errmsg("invalid input syntax for EmailAddr: \"%s\"", str)));
	}

	//result = (EmailAddr *) palloc(sizeof(EmailAddr));
    result = (EmailAddr *) palloc(VARHDRSZ + sizeof(char) * (strlen(str) + 1));
	SET_VARSIZE(result, VARHDRSZ + sizeof(char) * (strlen(str) + 1));
	//result->length = strlen(str);
	memcpy(result->content, str, sizeof(char) * (strlen(str)+1));
	PG_RETURN_POINTER(result);
}

/*
	Output Function.
 */
PG_FUNCTION_INFO_V1(email_out);

Datum
email_out(PG_FUNCTION_ARGS)
{
	EmailAddr  *email = (EmailAddr *) PG_GETARG_POINTER(0);
	char	   *result;

    result = psprintf("%s", email->content);
	PG_RETURN_CSTRING(result);
}

static int
email_cmp_internal(EmailAddr * a, EmailAddr * b)
{
	
	char * token1 = palloc(sizeof(char) * strlen(a->content));
	char * token2 = palloc(sizeof(char) * strlen(b->content));
	char *a_domain;
	char *a_local;
	char *b_domain;
	char *b_local;

	strcpy(token1, a->content);
	token1 = strtok(token1, "@");
	a_local = token1;

	token1 = strtok(NULL, "@");
	a_domain = token1;

	strcpy(token2, b->content);
	token2 = strtok(token2, "@");
	b_local = token2;

	token2 = strtok(NULL, "@");
    b_domain = token2;

	if(strcasecmp(a_domain, b_domain) == 0) {
		return strcasecmp(a_local, b_local);
	}
	else
	{
	    return strcasecmp(a_domain, b_domain);
	}

	return 0;
}

/*
	Function equal "=".
 */
PG_FUNCTION_INFO_V1(email_eq);

Datum
email_eq(PG_FUNCTION_ARGS)
{
	EmailAddr *a = (EmailAddr *) PG_GETARG_POINTER(0);
	EmailAddr *b = (EmailAddr *) PG_GETARG_POINTER(1);

	PG_RETURN_BOOL(email_cmp_internal(a, b) == 0);
}

/*
	Function greater than ">".
 */
PG_FUNCTION_INFO_V1(email_gt);

Datum
email_gt(PG_FUNCTION_ARGS)
{
	EmailAddr *a = (EmailAddr *) PG_GETARG_POINTER(0);
	EmailAddr *b = (EmailAddr *) PG_GETARG_POINTER(1);

	PG_RETURN_BOOL(email_cmp_internal(a, b) > 0);
}

/*
	Function about equal "~".
 */
PG_FUNCTION_INFO_V1(email_ae);

Datum
email_ae(PG_FUNCTION_ARGS)
{
	EmailAddr *a = (EmailAddr *) PG_GETARG_POINTER(0);
	EmailAddr *b = (EmailAddr *) PG_GETARG_POINTER(1);

	char *token1;
	char *token2;
	char * cp1 = palloc(sizeof(char) * strlen(a->content));
	char * cp2 = palloc(sizeof(char) * strlen(b->content));
	char *a_domain;
	char *a_local;
	char *b_domain;
	char *b_local;

	strcpy(cp1, a->content);
	token1 = strtok(cp1, "@");
	a_local = token1;

	token1 = strtok(NULL, "@");
	a_domain = token1;

	strcpy(cp2, b->content);
	token2 = strtok(cp2, "@");
	b_local = token2;

	token2 = strtok(NULL, "@");
    b_domain = token2;

	PG_RETURN_BOOL(strcasecmp(a_domain, b_domain) == 0);
}

/*
	Function not equal "<>".
 */
PG_FUNCTION_INFO_V1(email_ne);

Datum 
email_ne(PG_FUNCTION_ARGS)
{
	EmailAddr *a = (EmailAddr *) PG_GETARG_POINTER(0);
	EmailAddr *b = (EmailAddr *) PG_GETARG_POINTER(1);

	PG_RETURN_BOOL(email_cmp_internal(a, b) != 0);
}

/*
	Function greater than or equal ">=".
 */
PG_FUNCTION_INFO_V1(email_ge);

Datum
email_ge(PG_FUNCTION_ARGS)
{
	EmailAddr *a = (EmailAddr *) PG_GETARG_POINTER(0);
	EmailAddr *b = (EmailAddr *) PG_GETARG_POINTER(1);

	PG_RETURN_BOOL(email_cmp_internal(a, b) >= 0);
}

/*
	Function less than "<".
 */
PG_FUNCTION_INFO_V1(email_lt);

Datum 
email_lt(PG_FUNCTION_ARGS)
{
	EmailAddr *a = (EmailAddr *) PG_GETARG_POINTER(0);
	EmailAddr *b = (EmailAddr *) PG_GETARG_POINTER(1);

	PG_RETURN_BOOL(email_cmp_internal(a, b) < 0);
}

/*
	Function less than or equal "<=".
 */
PG_FUNCTION_INFO_V1(email_le);

Datum 
email_le(PG_FUNCTION_ARGS)
{
	EmailAddr *a = (EmailAddr *) PG_GETARG_POINTER(0);
	EmailAddr *b = (EmailAddr *) PG_GETARG_POINTER(1);

	PG_RETURN_BOOL(email_cmp_internal(a, b) <= 0);
}

/*
	Function not about equal "!~".
 */
PG_FUNCTION_INFO_V1(email_nae);

Datum 
email_nae(PG_FUNCTION_ARGS)
{
	EmailAddr *a = (EmailAddr *) PG_GETARG_POINTER(0);
	EmailAddr *b = (EmailAddr *) PG_GETARG_POINTER(1);

	char *token1;
	char *token2;
	char * cp1 = palloc(sizeof(char) * strlen(a->content));
	char * cp2 = palloc(sizeof(char) * strlen(b->content));
	char *a_domain;
	char *a_local;
	char *b_domain;
	char *b_local;

	strcpy(cp1, a->content);
	token1 = strtok(cp1, "@");
	a_local = token1;

	token1 = strtok(NULL, "@");
	a_domain = token1;

	strcpy(cp2, b->content);
	token2 = strtok(cp2, "@");
	b_local = token2;

	token2 = strtok(NULL, "@");
    b_domain = token2;

	PG_RETURN_BOOL(strcasecmp(a_domain, b_domain) != 0);
}

/*
	Support function.
 */
PG_FUNCTION_INFO_V1(email_cmp);

Datum 
email_cmp(PG_FUNCTION_ARGS)
{
	EmailAddr *a = (EmailAddr *) PG_GETARG_POINTER(0);
	EmailAddr *b = (EmailAddr *) PG_GETARG_POINTER(1);

	PG_RETURN_INT32(email_cmp_internal(a, b));
}


/*
	Hash function.
 */
 

 
PG_FUNCTION_INFO_V1(email_hash);

Datum hash_any(unsigned char * address, int length);


Datum
email_hash(PG_FUNCTION_ARGS)
{	
	EmailAddr  *email = (EmailAddr *) PG_GETARG_POINTER(0);
	char	   *address;

    int len = strlen(email->content);
	address = (char *) palloc(sizeof(char) * len);

    snprintf(address, sizeof(char) * len, "%s", email->content);
	
	Datum result = hash_any((unsigned char *) address, strlen(address));
	PG_RETURN_DATUM(result);
}
