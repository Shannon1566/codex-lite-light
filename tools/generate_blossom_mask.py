"""Generate a compact 1-bit mask from the official OpenAI Blossom asset."""

from pathlib import Path

from PIL import Image


ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "firmware" / "assets" / "openai-blossom.png"
OUTPUT = ROOT / "firmware" / "include" / "openai_blossom_mask.h"
SIZE = 84


def main() -> None:
    alpha = Image.open(SOURCE).convert("RGBA").getchannel("A")
    bounds = alpha.getbbox()
    if bounds is None:
        raise RuntimeError("Official Blossom asset has no visible pixels")
    logo = alpha.crop(bounds).resize((SIZE - 4, SIZE - 4), Image.Resampling.LANCZOS)
    canvas = Image.new("L", (SIZE, SIZE), 0)
    canvas.paste(logo, (2, 2))
    bits = [1 if value >= 96 else 0 for value in canvas.get_flattened_data()]
    packed = []
    for offset in range(0, len(bits), 8):
        byte = 0
        for bit_index, bit in enumerate(bits[offset : offset + 8]):
            byte |= bit << bit_index
        packed.append(byte)

    lines = [
        "#ifndef CODEX_LIGHT_OPENAI_BLOSSOM_MASK_H",
        "#define CODEX_LIGHT_OPENAI_BLOSSOM_MASK_H",
        "",
        "#include <Arduino.h>",
        "",
        f"constexpr uint8_t OPENAI_BLOSSOM_MASK_SIZE = {SIZE};",
        "const uint8_t OPENAI_BLOSSOM_MASK[] = {",
    ]
    for offset in range(0, len(packed), 16):
        lines.append("    " + ", ".join(f"0x{x:02X}" for x in packed[offset : offset + 16]) + ",")
    lines.extend(("};", "", "#endif", ""))
    OUTPUT.write_text("\n".join(lines), encoding="utf-8")


if __name__ == "__main__":
    main()
