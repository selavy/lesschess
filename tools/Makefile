CC=gcc
CFLAGS=
TARGET=gen_magic_tables

run: $(TARGET)
	./$(TARGET)
	mv magic_tables.* ..

$(TARGET): generate_magic_tables.c
	$(CC) -o $@ $(CFLAGS) $<

.PHONY: clean
clean:
	rm -rf $(TARGET)
