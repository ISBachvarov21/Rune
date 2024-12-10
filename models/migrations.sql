CREATE TABLE AttrTests (
	default TEXT DEFAULT 'Test default value',
	fk INTEGER FOREIGN KEY REFERENCES Posts(id),
	nullable INTEGER NOT NULL,
	pk INTEGER PRIMARY KEY
);

