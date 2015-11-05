-- add new columns to the tree metadata table
ALTER TABLE statehistory 
	ADD COLUMN creation	timestamp(6) DEFAULT now();

CREATE INDEX statehist_creation_idx ON statehistory(creation);

-- Load new functions
\i getStateChanges.sql

-- Load modified/improved functions
\i exportTree_func.sql
\i getTreeGroup_func.sql
