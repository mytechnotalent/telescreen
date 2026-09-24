"""Generate the TELESCREEN ASCII banner used by the course documents."""
import argparse

FONT = {
    "T": ["_____", "  |  ", "  |  ", "  |  ", "  |  "],
    "E": ["____ ", "|___ ", "|___ ", "|___ ", "____ "],
    "L": ["|    ", "|    ", "|    ", "|    ", "|___ "],
    "S": ["____ ", "|___ ", "___| ", "|___ ", "____ "],
    "C": [" ____", "/    ", "|    ", "\\____", " ____"],
    "R": ["____ ", "|__/ ", "|  \\ ", "|   \\", "|    "],
    "N": ["|\\  |", "| \\ |", "|  \\|", "|   |", "|   |"],
}


def _render(word: str) -> str:
    """
    Render a word as five ASCII rows.

    Parameters
    ----------
    word : str
        Word to render.

    Returns
    -------
    str
        Rendered banner text.
    """
    rows = [""] * 5
    for ch in word.upper():
        glyph = FONT.get(ch, ["?????"] * 5)
        for i in range(5):
            rows[i] += glyph[i] + " "
    return "\n".join(rows)


def main() -> int:
    """
    Print the banner for the supplied word.

    Parameters
    ----------
    None

    Returns
    -------
    int
        Zero on success.
    """
    ap = argparse.ArgumentParser()
    ap.add_argument("--word", default="TELESCREEN")
    args = ap.parse_args()
    print(_render(args.word))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
