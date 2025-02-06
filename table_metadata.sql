DROP TABLE IF EXISTS table_metadata;
CREATE TABLE table_metadata (
    table_schema TEXT,
    table_name TEXT,
    column_name TEXT,
    data_type TEXT,
    character_maximum_length INTEGER,
    numeric_precision INTEGER,
    numeric_scale INTEGER
);

INSERT INTO table_metadata (table_schema, table_name, column_name, data_type, character_maximum_length, numeric_precision, numeric_scale)
SELECT 
    table_schema,
    table_name,
    column_name,
    data_type,
    character_maximum_length,
    numeric_precision,
    numeric_scale
FROM 
    information_schema.columns
WHERE 
    table_schema NOT IN ('pg_catalog', 'information_schema')
ORDER BY 
    table_schema, table_name, ordinal_position;
