import psycopg2
import sys
import json

def get_query_cost(query, hypo_index_stmt):
    try:
        conn = psycopg2.connect(
            dbname="ecommerce",
            user="test",
            password="test",
            host="localhost",
            port="5432"
        )
        cur = conn.cursor()

        # Load HypoPG extension
        cur.execute("CREATE EXTENSION IF NOT EXISTS hypopg;")

        # Create hypothetical index
        cur.execute(f"""SELECT hypopg_create_index('{hypo_index_stmt}');""")

        # Run EXPLAIN to get cost
        cur.execute(f"EXPLAIN (FORMAT JSON) {query};")
        result = cur.fetchone()[0][0]
        # print(result)s

        # Extract final total cost
        final_cost = result['Plan']['Total Cost']
        print(final_cost)

        cur.close()
        conn.close()
    except Exception as e:
        print(f"Error: {e}")
        sys.exit(1)

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python optimizer.py '<query>' '<hypo_index_stmt>'")
        sys.exit(1)
    
    input_query = sys.argv[1]
    hypo_index_stmt = sys.argv[2]
    get_query_cost(input_query, hypo_index_stmt)
