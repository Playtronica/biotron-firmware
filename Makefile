IMAGE_NAME = plsdk-build-env
IMAGE_VERSION = v2
IMAGE = $(IMAGE_NAME):$(IMAGE_VERSION)

IMAGE_ID = $(shell docker images -q $(IMAGE))

PWD := $(shell pwd)

TIMESTAMP := $(shell date "+%s")

create_container: build_image
	docker rm -f $(IMAGE_NAME) || :;
	docker create -t -v $(PWD):/app/build --name $(IMAGE_NAME) $(IMAGE)
	docker start $(IMAGE_NAME)

build: create_container
	mkdir -p output
	echo $(TIMESTAMP)
	docker exec -e FLASH_ID_STARTUP=$(TIMESTAMP) $(IMAGE_NAME) bash -c "cd build/output ; cmake .. ; make;"
	$(call stop_and_rm_container)

build_image:
ifeq ($(IMAGE_ID),)
WORK_DIR = .build

build_image:
	mkdir -p $(WORK_DIR)/usr/local/src/pico
	docker build --file=Dockerfile -t $(IMAGE) $(WORK_DIR)
	rm -rf $(WORK_DIR)
.NOTPARALLEL: build_image
endif

.PHONY: all create_container run_container clean_cache build_image