package gameofgo.common;

public record GameRecord(
        String id,
        int boardSize,
        String blackPlayer,
        String whitePlayer,
        double blackScore,
        double whiteScore,
        long time
) {
}
