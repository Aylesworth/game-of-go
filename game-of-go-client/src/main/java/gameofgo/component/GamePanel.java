package gameofgo.component;

import gameofgo.common.Message;
import gameofgo.service.SocketService;
import javafx.geometry.Point2D;
import javafx.scene.canvas.Canvas;
import javafx.scene.canvas.GraphicsContext;
import javafx.scene.layout.BorderPane;
import javafx.scene.paint.Color;

public class GamePanel extends BorderPane {
    private static final Color BACKGROUND = Color.rgb(245, 183, 91);
    private static final Color LINE_COLOR = Color.BROWN;
    private static final double FULL_WIDTH = 720;
    private static final double PADDING = 40;

    private final int BOARD_SIZE = 19;
    private final double CELL_WIDTH = (FULL_WIDTH - 2 * PADDING) / (BOARD_SIZE - 1);
    private final double STONE_RADIUS = CELL_WIDTH * 0.45;
    private final int MY_COLOR;
    private boolean myTurn;
    private final SocketService socketService = SocketService.getInstance();

    private Canvas gameBoard;
    private GraphicsContext gc;
    private String lastPosition;
    private int lastColor;

    public GamePanel(int color) {
        MY_COLOR = color;
        myTurn = color == 1;
        setupGameBoard();
    }

    private void setupGameBoard() {
        gameBoard = new Canvas(FULL_WIDTH, FULL_WIDTH);
        gc = gameBoard.getGraphicsContext2D();

        gc.setFill(BACKGROUND);
        gc.fillRect(0, 0, FULL_WIDTH, FULL_WIDTH);

        gc.setStroke(LINE_COLOR);
        gc.setLineWidth(3);

        for (int i = 0; i < BOARD_SIZE; i++) {
            gc.strokeLine(PADDING + i * CELL_WIDTH, PADDING, PADDING + i * CELL_WIDTH, FULL_WIDTH - PADDING);
            gc.strokeLine(PADDING, PADDING + i * CELL_WIDTH, FULL_WIDTH - PADDING, PADDING + i * CELL_WIDTH);
        }

        gameBoard.setOnMouseClicked(event -> {
            if (!myTurn)
                return;

            double x = event.getX();
            double y = event.getY();

            if (x < PADDING || x > FULL_WIDTH - PADDING || y < PADDING || y > FULL_WIDTH - PADDING)
                return;

            String label = coordinatesToLabel(new Point2D(x, y));
            play(label, MY_COLOR);
            myTurn = !myTurn;

            socketService.send(new Message("MOVE", label));
        });

        socketService.on("MOVE", message -> {
            String label = message.payload().split("\n")[0];
            play(label, 1 - MY_COLOR);
            myTurn = !myTurn;
        });

        setLeft(gameBoard);
    }

    private void play(String positionLabel, int color) {
        putStone(positionLabel, color, true);
        if (lastPosition != null)
            putStone(lastPosition, lastColor, false);
        lastPosition = positionLabel;
        lastColor = color;
    }

    private void putStone(String positionLabel, int color, boolean withMarker) {
        Point2D coordinates = labelToCoordinates(positionLabel);

        gc.setFill(color == 0 ? Color.WHITE : Color.BLACK);
        gc.fillOval(coordinates.getX() - STONE_RADIUS, coordinates.getY() - STONE_RADIUS, 2 * STONE_RADIUS, 2 * STONE_RADIUS);

        if (withMarker) {
            gc.setStroke(color == 0 ? Color.BLACK : Color.WHITE);
            gc.setLineWidth(2);
            gc.strokeOval(coordinates.getX() - STONE_RADIUS / 2, coordinates.getY() - STONE_RADIUS / 2, STONE_RADIUS, STONE_RADIUS);
        }
    }

    private void removeStone(String positionLabel) {
        Point2D coordinates = labelToCoordinates(positionLabel);
        double x = coordinates.getX();
        double y = coordinates.getY();

        gc.setFill(BACKGROUND);
        gc.fillRect(x - STONE_RADIUS, y - STONE_RADIUS, 2 * STONE_RADIUS, 2 * STONE_RADIUS);

        gc.setStroke(LINE_COLOR);
        gc.strokeLine(x - STONE_RADIUS, y, x + STONE_RADIUS, y);
        gc.strokeLine(x, y - STONE_RADIUS, x, y + STONE_RADIUS);
    }

    private Point2D labelToCoordinates(String label) {
        int colChar = label.charAt(0);
        if (colChar >= 'J') colChar--;

        int col = colChar - 'A';
        int row = Integer.parseInt(label.substring(1)) - 1;

        double x = PADDING + col * CELL_WIDTH;
        double y = FULL_WIDTH - (PADDING + row * CELL_WIDTH);
        return new Point2D(x, y);
    }

    private String coordinatesToLabel(Point2D coordinates) {
        int col = (int) Math.round((coordinates.getX() - PADDING) / CELL_WIDTH);
        int row = BOARD_SIZE - 1 - (int) Math.round((coordinates.getY() - PADDING) / CELL_WIDTH);

        char colChar = (char) (col + 'A');
        if (colChar >= 'I') colChar++;

        return "" + colChar + (row + 1);
    }

    private record Position(int x, int y) {
    }
}
