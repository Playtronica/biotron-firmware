IMAGE_NAME = biotron-build-env
IMAGE_VERSION = v1
IMAGE = $(IMAGE_NAME):$(IMAGE_VERSION)

IMAGE_ID = $(shell docker images -q $(IMAGE))

PWD := $(shell pwd)

TIMESTAMP := $(shell date "+%s")



create_container: build_image
	docker rm -f $(IMAGE_NAME) || :;
	docker create -t -v $(PWD):/build --name $(IMAGE_NAME) $(IMAGE)
	docker start $(IMAGE_NAME)

build: create_container
	mkdir -p output
	echo $(TIMESTAMP)
	docker exec -e FLASH_ID_STARTUP=$(TIMESTAMP) $(IMAGE_NAME) bash -c "cd /build/output ; cmake .. ; make;"
	$(call stop_and_rm_container)

build_image:
ifeq ($(IMAGE_ID),)
WORK_DIR = .build

build_image:
	mkdir -p $(WORK_DIR)/usr/local/src/pico
	git clone https://github.com/raspberrypi/pico-sdk/ $(WORK_DIR)/usr/local/src/pico/pico-sdk
	git clone https://github.com/raspberrypi/pico-extras $(WORK_DIR)/usr/local/src/pico/pico-extras
	git clone https://github.com/raspberrypi/pico-examples $(WORK_DIR)/usr/local/src/pico/pico-examples
	cd $(WORK_DIR)/usr/local/src/pico/pico-sdk && git submodule update --init
	docker build --file=Dockerfile -t $(IMAGE) $(WORK_DIR)
	rm -rf $(WORK_DIR)
.NOTPARALLEL: build_image
endif

.PHONY: all create_container run_container clean_cache build_image