gdb ./build/astral_engine/astral_engine \
    --ex "set environment __NV_PRIME_RENDER_OFFLOAD 1" \
    --ex "set environment __GLX_VENDOR_LIBRARY_NAME nvidia" \
    --ex run

