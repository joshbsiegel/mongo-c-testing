#include <stdio.h>
#include <mongoc/mongoc.h>
#include <pthread.h>

static pthread_mutex_t mutex;

#define NUM_THREADS 1000

static void * threadScramAuth(void *data) {
   mongoc_client_pool_t *pool = data;
   mongoc_client_t *client;
   bson_t ping = BSON_INITIALIZER;
   bson_error_t error;
   bool r;

   BSON_APPEND_INT32 (&ping, "ping", 1);

   client = mongoc_client_pool_pop (pool);
   r = mongoc_client_command_simple (
         client, "admin", &ping, NULL, NULL, &error);
   mongoc_client_pool_push(pool, client);
}

int main()
{

   mongoc_client_t *client = NULL;
   mongoc_database_t *database = NULL;
   mongoc_client_pool_t *pool;
   bson_error_t error;
   const char *uri_string = "mongodb://127.0.0.1/";
   mongoc_uri_t *uri = NULL;
   const char *authuristr;
   bson_t roles;
   const bson_t *doc;
   void *ret;
   pthread_t threads[NUM_THREADS];
   mongoc_init ();

   bson_init (&roles);

   uri = mongoc_uri_new_with_error (uri_string, &error);
   if (!uri) {
      fprintf (stderr,
               "failed to parse URI: %s\n"
               "error message:       %s\n",
               uri_string,
               error.message);
      return EXIT_FAILURE;
   }

   client = mongoc_client_new_from_uri (uri);
   if (!client) {
      return EXIT_FAILURE;
   }
   
   mongoc_uri_destroy(uri);

   mongoc_client_set_error_api (client, 2);

   database = mongoc_client_get_database (client, "test");

   BCON_APPEND (&roles, "0", "{", "role", "root", "db", "admin", "}");

   mongoc_database_add_user (database, "user,=", "pass", &roles, NULL, &error);

   mongoc_database_destroy (database);

   mongoc_client_destroy (client);

   char * uri_str = bson_strdup_printf("mongodb://user,=:pass@127.0.0.1/test?appname=scram-example&maxPoolSize=%d&authMechanism=SCRAM-SHA-1", NUM_THREADS);

   uri = mongoc_uri_new_with_error (uri_str, &error);

   bson_free(uri_str);

   if (!uri) {
      fprintf (stderr,
               "failed to parse URI\n"
               "error message: %s\n",
               error.message);
      return EXIT_FAILURE;
   }
   pool = mongoc_client_pool_new (uri);

   mongoc_client_pool_set_error_api (pool, 2);

   int64_t before = bson_get_monotonic_time();
   for (int i = 0; i < NUM_THREADS; ++i) {
      pthread_create(&threads[i], NULL, threadScramAuth, pool);
   }

   for (int i = 0; i < NUM_THREADS; ++i) {
      pthread_join (threads[i], &ret);
   }
   int64_t after = bson_get_monotonic_time();

   printf("total time taken: %"PRId64, after - before);

   return EXIT_SUCCESS;
}