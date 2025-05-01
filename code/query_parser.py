import sqlparse
import os
import sys
from sqlparse.sql import IdentifierList, Identifier, Token, Function, Parenthesis
from sqlparse.tokens import Keyword, DML, DDL, Whitespace, Punctuation, Name, Wildcard

# Define common SQL functions and keywords to exclude from column detection
SQL_FUNCTIONS = {
    'SUM', 'COUNT', 'AVG', 'MIN', 'MAX', 'RANK', 'DENSE_RANK', 'ROW_NUMBER',
    'FIRST_VALUE', 'LAST_VALUE', 'LAG', 'LEAD', 'NTILE', 'STDDEV', 'VARIANCE',
    'EXTRACT', 'TO_CHAR', 'TO_DATE', 'COALESCE', 'NULLIF', 'CASE', 'CAST',
    'ROUND', 'TRUNC', 'CONCAT', 'SUBSTR', 'LENGTH', 'UPPER', 'LOWER', 'TRIM',
    'CURRENT_DATE', 'CURRENT_TIME', 'CURRENT_TIMESTAMP', 'NOW'
}

SQL_KEYWORDS = {
    'SELECT', 'FROM', 'WHERE', 'GROUP', 'BY', 'HAVING', 'ORDER', 'LIMIT',
    'JOIN', 'INNER', 'LEFT', 'RIGHT', 'FULL', 'OUTER', 'ON', 'AS', 'AND',
    'OR', 'NOT', 'IN', 'EXISTS', 'BETWEEN', 'LIKE', 'IS', 'NULL', 'TRUE',
    'FALSE', 'ASC', 'DESC', 'DISTINCT', 'ALL', 'UNION', 'INTERSECT', 'EXCEPT',
    'WITH', 'OVER', 'PARTITION', 'ROWS', 'RANGE', 'PRECEDING', 'FOLLOWING',
    'UNBOUNDED', 'CURRENT', 'ROW'
}

def extract_relations_and_attributes(sql_query):
    """
    Extract relations (tables) and their attributes (columns) from a SQL query.
    
    Args:
        sql_query (str): The SQL query to analyze
        
    Returns:
        dict: A dictionary mapping relation names to their used attributes
    """
    # Parse the SQL query
    try:
        parsed = sqlparse.parse(sql_query)[0]
    except IndexError:
        print("Error: Unable to parse SQL query")
        return {}
    
    # Initialize dictionary to store relations and their attributes
    relations = {}
    
    # Process the SQL statement based on its type
    if parsed.token_first().ttype == DML and parsed.token_first().value.upper() == 'INSERT':
        process_insert_statement(parsed, relations)
    elif parsed.token_first().ttype == DDL and parsed.token_first().value.upper() == 'CREATE':
        process_create_statement(parsed, relations)
    else:
        process_select_statement(parsed, relations)
    
    # Print the results
    for table_name, columns in relations.items():
        lower_columns = [col.lower() for col in columns]
        print(f"{table_name.lower()}: {lower_columns}")
    
    return relations

def process_create_statement(parsed, relations):
    """Process a CREATE TABLE statement to extract tables and columns."""
    table_name = None
    for i, token in enumerate(parsed.tokens):
        # Find the table name after CREATE TABLE
        if token.ttype == Keyword and token.value.upper() == 'TABLE':
            j = i + 1
            while j < len(parsed.tokens):
                next_token = parsed.tokens[j]
                if next_token.is_whitespace:
                    j += 1
                    continue
                
                if isinstance(next_token, Identifier):
                    table_name = next_token.get_real_name()
                    relations[table_name] = []
                    break
                j += 1
            break
    
    # Extract column definitions from the parenthesis
    if table_name:
        for token in parsed.tokens:
            if isinstance(token, Parenthesis):
                for item in token.tokens:
                    if isinstance(item, IdentifierList):
                        for subitem in item.tokens:
                            if isinstance(subitem, Identifier):
                                col_name = subitem.get_real_name()
                                if col_name and col_name not in relations[table_name]:
                                    relations[table_name].append(col_name)

def process_insert_statement(parsed, relations):
    """Process an INSERT statement to extract tables and columns."""
    # Find target table (the one we're inserting into)
    target_table = None
    
    for i, token in enumerate(parsed.tokens):
        if token.ttype == Keyword and token.value.upper() == 'INTO':
            # Find the table name after INTO
            j = i + 1
            while j < len(parsed.tokens):
                next_token = parsed.tokens[j]
                if next_token.is_whitespace:
                    j += 1
                    continue
                
                if isinstance(next_token, Identifier):
                    target_table = next_token.get_real_name()
                    relations[target_table] = []
                    break
                j += 1
            break
    
    if not target_table:
        return
    
    # Find target columns (if specified)
    for token in parsed.tokens:
        if isinstance(token, Parenthesis) and target_table:
            for item in token.tokens:
                if isinstance(item, IdentifierList):
                    for subitem in item.tokens:
                        if isinstance(subitem, Identifier):
                            col_name = subitem.get_real_name()
                            if col_name and col_name not in relations[target_table]:
                                relations[target_table].append(col_name)
            break
    
    # Find and process the SELECT part if this is an INSERT INTO ... SELECT
    select_found = False
    for token in parsed.tokens:
        if token.ttype == DML and token.value.upper() == 'SELECT':
            select_found = True
            break
    
    if select_found:
        # Process the SELECT part
        process_select_statement(parsed, relations)

def process_select_statement(parsed, relations):
    """Process a SELECT statement to extract tables and columns."""
    # Track table aliases
    aliases = {}
    
    # First pass: identify all tables and their aliases
    extract_tables(parsed, relations, aliases)
    
    # Second pass: identify columns and associate with tables
    extract_columns(parsed, relations, aliases)

def extract_tables(parsed, relations, aliases):
    """Extract table names and aliases from the SQL statement."""
    for token in parsed.tokens:
        # Skip irrelevant tokens
        if token.is_whitespace or token.ttype == Punctuation:
            continue
        
        # Process FROM and JOIN clauses to find tables
        if token.ttype == Keyword and token.value.upper() in ('FROM', 'JOIN'):
            i = parsed.token_index(token)
            if i + 1 < len(parsed.tokens):
                # Get the next non-whitespace token
                next_token = parsed.tokens[i + 1]
                while next_token.is_whitespace and i + 2 < len(parsed.tokens):
                    i += 1
                    next_token = parsed.tokens[i + 1]
                
                # Extract table name from FROM/JOIN clause
                if isinstance(next_token, Identifier):
                    process_table_identifier(next_token, relations, aliases)
                elif isinstance(next_token, IdentifierList):
                    for ident in next_token.get_identifiers():
                        if isinstance(ident, Identifier):
                            process_table_identifier(ident, relations, aliases)
        
        # Recursively process token lists (e.g., subqueries)
        if hasattr(token, 'tokens'):
            extract_tables(token, relations, aliases)

def process_table_identifier(identifier, relations, aliases):
    """Process a table identifier and extract its name and potential alias."""
    table_name = identifier.get_real_name()
    alias = identifier.get_alias()
    
    # Add table to relations if it's not already there
    if table_name and table_name not in relations:
        relations[table_name] = []
    
    # Record the alias -> table mapping
    if alias:
        aliases[alias] = table_name

def extract_columns(parsed, relations, aliases):
    """Extract column references from the SQL statement."""
    def process_token_for_columns(token):
        """Process a token to extract column references."""
        # Handle identifiers that might be columns
        if isinstance(token, Identifier):
            column = token.get_real_name()
            table_ref = token.get_parent_name()
            
            # Skip if this is a SQL function or keyword
            if column.upper() in SQL_FUNCTIONS or column.upper() in SQL_KEYWORDS:
                return
            
            # Skip if this is a single-letter alias (like 's', 'e', 'a')
            if len(column) == 1 and column.isalpha():
                return
            
            # Handle table.column format
            if table_ref:
                # Check if it's an alias
                actual_table = aliases.get(table_ref, table_ref)
                if actual_table in relations and column not in relations[actual_table]:
                    relations[actual_table].append(column)
            # Handle standalone column name (ambiguous)
            elif column and not column.isdigit():
                # For standalone columns, we need to be more selective
                # Don't add common SQL functions/keywords as columns
                for table in relations:
                    if column not in relations[table]:
                        relations[table].append(column)
        
        # Handle wildcards (SELECT * FROM...)
        elif token.ttype == Wildcard:
            # When * is used, we can't determine specific columns
            # We'll just note this in our output
            for table in relations:
                if '*' not in relations[table]:
                    relations[table].append('*')
        
        # Recursively process tokens
        if hasattr(token, 'tokens'):
            for subtoken in token.tokens:
                process_token_for_columns(subtoken)
    
    # Process all tokens
    # for token in parsed.tokens:
    #     process_token_for_columns(token)

    select_seen = False
    for token in parsed.tokens:
        if token.ttype is DML and token.value.upper() == 'SELECT':
            select_seen = True
            continue
        if select_seen:
            if token.ttype is Keyword and token.value.upper() == 'FROM':
                select_seen = False
                continue
            continue
        
        process_token_for_columns(token)

    
    # Clean up potential aliases in column lists
    for table in relations:
        # Filter out single-character aliases
        relations[table] = [
            col for col in relations[table] 
            if len(col) > 1 or not col.isalpha()
        ]
        
        # Filter out SQL functions and keywords
        relations[table] = [
            col for col in relations[table]
            if col.upper() not in SQL_FUNCTIONS and col.upper() not in SQL_KEYWORDS
        ]

def main():
    # Example SQL query
    if len(sys.argv) != 2:
        print("Usage: python query_parser.py <sql_query_file>")
        sys.exit(1)

    sql_query_file = sys.argv[1]

    if not os.path.exists(sql_query_file):
        print(f"Error: File '{sql_query_file}' does not exist.")
        sys.exit(1)

    with open(sql_query_file, 'r') as file:
        sql_query = file.read()

    # Call the function to extract relations and attributes
    result = extract_relations_and_attributes(sql_query)
    
if __name__ == "__main__":
    main()