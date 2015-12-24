MAKEFLAGS += -Rr
Q=@

APP = nanopb_json
APP_PATH := $(shell pwd)

CC = gcc
PROTOC = generator-bin/protoc.exe

BUILD = $(APP_PATH)/build
TARGET = $(BUILD)/$(APP)

src-to-obj = $(addprefix $(BUILD)/obj/,$(addsuffix $3,$(basename $(filter %$2,$1))))

SRCS += $(addprefix $(APP_PATH)/,\
  test.proto \
  examples/json/json.c \
  pb_common.c \
  pb_decode.c \
  pb_encode.c \
  pb_json.c \
  thirdparty/jsmn.c \
)

INCLUDES += $(APP_PATH)

nanopb_srcs = $(patsubst %.proto,%.pb.c,$(filter %.proto,$(SRCS))) $(patsubst %.proto,%.pb.h,$(filter %.proto,$(SRCS)))

objects +=  $(call src-to-obj,$(nanopb_srcs),.c,.c.o) $(call src-to-obj,$(SRCS),.c,.c.o)
dep-files += $(call src-to-obj,$(nanopb_srcs),.c,.c.d) $(call src-to-obj,$(SRCS),.c,.c.d)

CFLAGS += -std=c11
CFLAGS += $(addprefix -I,$(INCLUDES))

LDFLAGS += -Wl,-Map=$(TARGET).map

.PHONY: nanopb
nanopb:
	$(MAKE) -C generator/proto

.PHONY: all
all: $(TARGET).exe $(protobuf-generated)

.PHONY: clean
clean:
	$(Q)rm -rf $(BUILD) $(nanopb_srcs)

$(TARGET).exe: $(objects)
	$(Q)$(CC) $^ $(LDFLAGS) -o $@

$(BUILD)/obj/%.c.o: %.c $(BUILD)/obj/%.c.d
	@echo CC $(notdir $<)
	$(Q)mkdir -p $(dir $@)
	$(Q)$(CC) $(CFLAGS) -c $< -o $@

.PRECIOUS: $(BUILD)/obj/%.d

ifeq ($(words $(findstring $(MAKECMDGOALS),clean)), 0)
-include $(dep-files)
endif

$(BUILD)/obj/%.c.d: %.c
	$(Q)mkdir -p $(dir $@)
	$(Q)$(CC) $(CFLAGS) -M -MT $(patsubst %.c,$(BUILD)/obj/%.o,$<) $< -MF $@

.PRECIOUS: %.pb.c %.pb.h	
%.pb.c %.pb.h: %.proto
	@echo PROTOC $(notdir $<)
	@echo NANOPB_GEN $(notdir $(<:.proto=.pb.c))
	$(Q)$(PROTOC) -I$(dir $<) --plugin=protoc-gen-nanopb=generator\protoc-gen-nanopb.bat --nanopb_out=$(dir $<) $<

