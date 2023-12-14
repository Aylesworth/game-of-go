package gameofgo.common;

public record GameReplay(
        int boardSize,
        int color,
        String log,
        double blackScore,
        double whiteScore,
        String[] blackTerritory,
        String[] whiteTerritory
) {
}
