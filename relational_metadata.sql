DROP TABLE IF EXISTS relational_metadata;
CREATE TABLE relational_metadata (
    table_schema TEXT,
    table_name TEXT,
    column_name TEXT,
    constraint_name TEXT,
    foreign_table_schema TEXT,
    foreign_table_name TEXT,
    foreign_column_name TEXT
);

INSERT INTO relational_metadata (table_schema, table_name, column_name, constraint_name, foreign_table_schema, foreign_table_name, foreign_column_name)
SELECT 
    tc.table_schema,
    tc.table_name,
    kcu.column_name,
    tc.constraint_name,
    ccu.table_schema AS foreign_table_schema,
    ccu.table_name AS foreign_table_name,
    ccu.column_name AS foreign_column_name
FROM 
    information_schema.table_constraints AS tc
JOIN 
    information_schema.key_column_usage AS kcu
      ON tc.constraint_name = kcu.constraint_name
      AND tc.table_schema = kcu.table_schema
JOIN 
    information_schema.constraint_column_usage AS ccu
      ON ccu.constraint_name = tc.constraint_name
      AND ccu.table_schema = tc.table_schema
WHERE 
    tc.constraint_type = 'FOREIGN KEY'
ORDER BY 
    tc.table_schema, tc.table_name;
