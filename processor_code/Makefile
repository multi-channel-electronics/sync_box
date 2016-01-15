CC = sdcc
CFLAGS = --std-sdcc99 --model-large
LDFLAGS = --out-fmt-ihx --xram-size 0x400 --xram-loc 0 \
	 --iram-size 0x256 --model-large --debug

BUILD = build
OUTPUT_HEX = $(BUILD)/SyncoCmd.hex
HEADERS = SyncoCmd.h
SRCS = SyncoCmd.c cmddict.c piolib.c sio.c strtoul.c
OBJECTS = $(patsubst %.c, %.rel, $(SRCS))
OBJECTS_BUILD = $(patsubst %, $(BUILD)/%, $(OBJECTS))

default: $(OUTPUT_HEX)

$(OUTPUT_HEX): $(OBJECTS_BUILD)
	sdcc $(LDFLAGS) $(OBJECTS_BUILD) -o $(OUTPUT_HEX)

$(BUILD)/%.rel: %.c $(HEADERS) $(BUILD)
	sdcc $(CFLAGS) $< -c -o $@

$(BUILD):
	-mkdir $(BUILD)

clean:
	rm -r $(BUILD)
