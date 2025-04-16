import sqlparse
import os
from sqlparse.sql import IdentifierList, Identifier, Token, Function, Parenthesis
from sqlparse.tokens import Keyword, DML, DDL, Whitespace, Punctuation, Name
import sys

def extract_relations_and_attributes(sql_query):
    """
    Extract relations (tables) and their attributes (columns) from a SQL query.
    
    Args:
        sql_query (str): The SQL query to analyze
        
    Returns:
        dict: A dictionary mapping relation names to their used attributes
    """
    # Parse the SQL query
    parsed = sqlparse.parse(sql_query)[0]
    
    # Initialize dictionaries to store relations and their attributes
    relations = {}
    aliases = {}
    
    # Handle CREATE TABLE statements
    if parsed.token_first().ttype == DDL and parsed.token_first().value.upper() == 'CREATE':
        table_name = None
        for token in parsed.tokens:
            # Find the table name after CREATE TABLE
            if token.ttype == Keyword and token.value.upper() == 'TABLE':
                i = parsed.token_index(token) + 1
                while i < len(parsed.tokens):
                    next_token = parsed.tokens[i]
                    if next_token.is_whitespace:
                        i += 1
                        continue
                    
                    if isinstance(next_token, Identifier):
                        table_name = next_token.get_real_name()
                        relations[table_name] = []
                        break
                    i += 1
            
            # Extract column definitions from the parenthesis
            if isinstance(token, Parenthesis) and table_name:
                for item in token.tokens:
                    if isinstance(item, IdentifierList):
                        for subitem in item.tokens:
                            if isinstance(subitem, Identifier):
                                col_name = subitem.get_real_name()
                                if col_name not in relations[table_name]:
                                    relations[table_name].append(col_name)
    
    # Handle INSERT statements
    elif parsed.token_first().ttype == DML and parsed.token_first().value.upper() == 'INSERT':
        table_name = None
        columns = []
        
        for token in parsed.tokens:
            # Find the table name after INSERT INTO
            if token.ttype == Keyword and token.value.upper() == 'INTO':
                i = parsed.token_index(token) + 1
                while i < len(parsed.tokens):
                    next_token = parsed.tokens[i]
                    if next_token.is_whitespace:
                        i += 1
                        continue
                    
                    if isinstance(next_token, Identifier) or isinstance(next_token, Function):
                        table_name = next_token.get_real_name()
                        relations[table_name] = []
                        break
                    i += 1
            
            # Extract column names from parenthesis after table name
            if isinstance(token, Parenthesis) and table_name and not columns:
                for item in token.tokens:
                    if isinstance(item, IdentifierList):
                        for subitem in item.tokens:
                            if isinstance(subitem, Identifier):
                                col_name = subitem.get_real_name()
                                columns.append(col_name)
                                if col_name not in relations[table_name]:
                                    relations[table_name].append(col_name)
    
    # Handle SELECT statements and other DML
    else:
        # Identify table names and aliases
        for token in parsed.tokens:
            # Skip whitespace and punctuation
            if token.is_whitespace or token.ttype == Punctuation:
                continue
                
            # Check for FROM or JOIN clauses
            if token.ttype == Keyword and token.value.upper() in ('FROM', 'JOIN'):
                # The next token after FROM/JOIN should contain the table name
                i = parsed.token_index(token) + 1
                while i < len(parsed.tokens):
                    next_token = parsed.tokens[i]
                    if next_token.is_whitespace:
                        i += 1
                        continue
                        
                    # Extract table names
                    if isinstance(next_token, Identifier):
                        table_name = next_token.get_real_name()
                        alias = next_token.get_alias()
                        relations[table_name] = []
                        if alias:
                            aliases[alias] = table_name
                    elif isinstance(next_token, IdentifierList):
                        for identifier in next_token.get_identifiers():
                            if isinstance(identifier, Identifier):
                                table_name = identifier.get_real_name()
                                alias = identifier.get_alias()
                                relations[table_name] = []
                                if alias:
                                    aliases[alias] = table_name
                    break
                    i += 1
                    
        # Extract SELECT column list
        select_columns = []
        for token in parsed.tokens:
            if token.ttype == DML and token.value.upper() == 'SELECT':
                i = parsed.token_index(token) + 1
                while i < len(parsed.tokens):
                    next_token = parsed.tokens[i]
                    if next_token.is_whitespace:
                        i += 1
                        continue
                    
                    if isinstance(next_token, IdentifierList):
                        for identifier in next_token.get_identifiers():
                            if isinstance(identifier, Identifier):
                                select_columns.append(identifier)
                    elif isinstance(next_token, Identifier):
                        select_columns.append(next_token)
                    break
                    i += 1
        
        # Process selected columns
        for col in select_columns:
            column = col.get_real_name()
            table_reference = col.get_parent_name()
            
            # If we have a table reference like "table.column"
            if table_reference:
                # Check if it's an alias
                if table_reference in aliases:
                    actual_table = aliases[table_reference]
                    if actual_table in relations and column not in relations[actual_table]:
                        relations[actual_table].append(column)
                # Or a direct table reference
                elif table_reference in relations and column not in relations[table_reference]:
                    relations[table_reference].append(column)
            # No table reference - could be an ambiguous column
            else:
                # Add to all possible tables as an option
                for table in relations:
                    if column not in relations[table]:
                        relations[table].append(column)
        
        # Process other attribute references in the query (WHERE, JOIN conditions, etc.)
        def process_tokens(token_list):
            for token in token_list:
                if hasattr(token, 'tokens'):
                    process_tokens(token.tokens)
                
                # Look for identifiers that could be columns
                if isinstance(token, Identifier):
                    column = token.get_real_name()
                    table_reference = token.get_parent_name()
                    
                    # If we have a table reference like "table.column"
                    if table_reference:
                        # Check if it's an alias
                        if table_reference in aliases:
                            actual_table = aliases[table_reference]
                            if actual_table in relations and column not in relations[actual_table]:
                                relations[actual_table].append(column)
                        # Or a direct table reference
                        elif table_reference in relations and column not in relations[table_reference]:
                            relations[table_reference].append(column)
        
        # Process all tokens to find column references
        process_tokens(parsed.tokens)
    
    for table, columns in relations.items():
        print(f"{table}, {columns}")

    return relations

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