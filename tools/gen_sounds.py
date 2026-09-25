"""効果音を合成して resources/sounds/ に WAV で書き出すスクリプト。

プロトタイプ(ブラウザのシンセ)で鳴らしていた音を、同じ周波数・波形で作る。
Python の標準ライブラリだけで動く(追加のインストール不要)。

使い方:
  python tools/gen_sounds.py
"""

import math
import pathlib
import struct
import wave

ROOT = pathlib.Path(__file__).resolve().parent.parent
OUT_DIR = ROOT / "resources" / "sounds"
SAMPLE_RATE = 22050
VOLUME = 0.5  # 全体の音量(クリップしないよう控えめ)


# --- 波形(phase は 0〜1 の周期内の位置) ---
def square(phase: float) -> float:
    return 1.0 if phase % 1.0 < 0.5 else -1.0


def triangle(phase: float) -> float:
    return 4.0 * abs(phase % 1.0 - 0.5) - 1.0


def sawtooth(phase: float) -> float:
    return 2.0 * (phase % 1.0) - 1.0


def sine(phase: float) -> float:
    return math.sin(2.0 * math.pi * phase)


def new_buffer(seconds: float) -> list:
    return [0.0] * int(SAMPLE_RATE * seconds)


def add_tone(buffer: list, start: float, duration: float, freq: float, wave_func, gain: float,
             attack: float = 0.005, release: float = 0.05) -> None:
    """start 秒から duration 秒、音を足す。立ち上がり attack、終わりは release 秒かけて消す"""
    first = int(start * SAMPLE_RATE)
    count = int(duration * SAMPLE_RATE)
    for i in range(count):
        index = first + i
        if index >= len(buffer):
            break
        t = i / SAMPLE_RATE
        envelope = min(1.0, t / attack) if attack > 0 else 1.0
        remaining = duration - t
        if remaining < release:
            envelope *= max(0.0, remaining / release)
        buffer[index] += gain * envelope * wave_func(freq * t)


def add_echo(buffer: list, delay: float, feedback: float, repeats: int) -> list:
    """残響の代わりに、少しずつ小さくなるこだまを重ねる"""
    shift = int(delay * SAMPLE_RATE)
    result = buffer + [0.0] * (shift * repeats)
    for r in range(1, repeats + 1):
        gain = feedback ** r
        for i, value in enumerate(buffer):
            result[i + shift * r] += value * gain
    return result


def save(name: str, buffer: list) -> None:
    peak = max(1e-6, max(abs(v) for v in buffer))
    scale = VOLUME / peak if peak > 1.0 else VOLUME
    frames = b"".join(struct.pack("<h", int(max(-1.0, min(1.0, v * scale)) * 32767)) for v in buffer)
    path = OUT_DIR / f"{name}.wav"
    with wave.open(str(path), "wb") as file:
        file.setnchannels(1)
        file.setsampwidth(2)
        file.setframerate(SAMPLE_RATE)
        file.writeframes(frames)
    print(f"{path.relative_to(ROOT)} ({len(frames) // 1024} KB)")


def make_rotate() -> list:
    """タイルを回した: 矩形波 430Hz を 0.08 秒、0.02 秒遅れて三角波 860Hz を重ねる"""
    buffer = new_buffer(0.12)
    add_tone(buffer, 0.0, 0.08, 430, square, 0.35, release=0.03)
    add_tone(buffer, 0.02, 0.08, 860, triangle, 0.5, release=0.04)
    return buffer


def make_goal(freq: float) -> list:
    """ゴールが点灯した: 三角波 0.22 秒。点灯数に応じて音が高くなる"""
    buffer = new_buffer(0.26)
    add_tone(buffer, 0.0, 0.22, freq, triangle, 0.9, release=0.12)
    return buffer


def make_clear() -> list:
    """クリア: 5 音を 0.09 秒間隔で重ね、オクターブ上を薄く重ねる"""
    notes = [523, 659, 784, 1047, 1319]
    buffer = new_buffer(0.09 * len(notes) + 0.5)
    for i, freq in enumerate(notes):
        add_tone(buffer, i * 0.09, 0.45, freq, triangle, 0.6, release=0.3)
        add_tone(buffer, i * 0.09, 0.45, freq * 2, sine, 0.15, release=0.3)
    return buffer


def make_restore() -> list:
    """色が戻る: 4 音を 0.12 秒間隔で、長めの残響"""
    notes = [392, 494, 587, 784]
    buffer = new_buffer(0.12 * len(notes) + 0.8)
    for i, freq in enumerate(notes):
        add_tone(buffer, i * 0.12, 0.7, freq, sine, 0.6, attack=0.02, release=0.5)
        add_tone(buffer, i * 0.12, 0.7, freq * 2, triangle, 0.12, attack=0.02, release=0.5)
    return add_echo(buffer, 0.18, 0.45, 4)


def make_ending() -> list:
    """エンディング: 6 音を 0.12 秒間隔で、1.4 秒の余韻"""
    notes = [523, 659, 784, 988, 1175, 1568]
    buffer = new_buffer(0.12 * len(notes) + 1.4)
    for i, freq in enumerate(notes):
        add_tone(buffer, i * 0.12, 1.4, freq, sine, 0.55, attack=0.02, release=1.0)
        add_tone(buffer, i * 0.12, 1.4, freq * 2, triangle, 0.1, attack=0.02, release=1.0)
    return add_echo(buffer, 0.22, 0.4, 3)


def make_blocked() -> list:
    """回せないタイル: 矩形波 180Hz を 0.12 秒"""
    buffer = new_buffer(0.14)
    add_tone(buffer, 0.0, 0.12, 180, square, 0.35, release=0.05)
    return buffer


def make_reset() -> list:
    """やりなおし: のこぎり波 300Hz を 0.15 秒"""
    buffer = new_buffer(0.17)
    add_tone(buffer, 0.0, 0.15, 300, sawtooth, 0.35, release=0.07)
    return buffer


def main() -> None:
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    save("rotate", make_rotate())
    for i, freq in enumerate([523, 587, 659, 784, 880, 988, 1175]):
        save(f"goal_{i}", make_goal(freq))
    save("clear", make_clear())
    save("restore", make_restore())
    save("ending", make_ending())
    save("blocked", make_blocked())
    save("reset", make_reset())


if __name__ == "__main__":
    main()
