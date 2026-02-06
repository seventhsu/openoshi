# OpenOshi CH32V003 firmware

To pull the `ch32fun` SDK and compile `minichlink`, the flasher tool:
```bash
git submodule update --init --recursive
cd ch32fun/minichlink
make
```

To build the board firmware, creating images and build artifacts in `src/`:
```bash
make -C src
```