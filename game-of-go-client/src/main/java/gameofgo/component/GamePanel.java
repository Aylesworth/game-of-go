package gameofgo.component;

import gameofgo.common.Message;
import gameofgo.service.SocketService;
import javafx.geometry.Point2D;
import javafx.scene.canvas.Canvas;
import javafx.scene.canvas.GraphicsContext;
import javafx.scene.layout.BorderPane;
import javafx.scene.paint.Color;

public class GamePanel extends BorderPane {
    private static final double FULL_WIDTH = 720;
    private static final double PADDING = 40;
    private final int BOARD_SIZE = 13;
    private final double CELL_WIDTH = (FULL_WIDTH - 2 * PADDING) / (BOARD_SIZE - 1);
    private final double STONE_RADIUS = CELL_WIDTH * 0.45;
    private final int myColor;
    private boolean myTurn;
    private final SocketService socketService = SocketService.getInstance();

    private Canvas gameBoard;
    private GraphicsContext gc;

    public GamePanel(int color) {
        myColor = color;
        myTurn = color == 1;
        setupGameBoard();
    }

    private void setupGameBoard() {
        gameBoard = new Canvas(FULL_WIDTH, FULL_WIDTH);
        gc = gameBoard.getGraphicsContext2D();

        gc.setFill(Color.GREENYELLOW);
        gc.fillRect(0, 0, FULL_WIDTH, FULL_WIDTH);

        gc.setStroke(Color.BROWN);
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

            String label = new Position(new Point2D(x, y)).toLabel();
            play(label, myColor);
            myTurn = !myTurn;

            socketService.send(new Message("MOVE", label));
        });

        socketService.on("MOVE", message -> {
            String label = message.payload().split("\n")[0];
            play(label, 1 - myColor);
            myTurn = !myTurn;
        });

        setLeft(gameBoard);
    }

    private void play(String positionLabel, int color) {
        Point2D point = new Position(positionLabel).toPoint();

        gc.setFill(color == 0 ? Color.WHITE : Color.BLACK);
        gc.fillOval(point.getX() - STONE_RADIUS, point.getY() - STONE_RADIUS, 2 * STONE_RADIUS, 2 * STONE_RADIUS);
    }

    private class Position {
        private int row;
        private int col;

        public Position(int row, int col) {
            this.row = row;
            this.col = col;
        }

        public Position(String label) {
            int colChar = label.charAt(0);
            if (colChar >= 'J') colChar--;

            this.col = colChar - 'A';
            this.row = Integer.parseInt(label.substring(1)) - 1;
        }

        public Position(Point2D point) {
            this.col = (int) Math.round((point.getX() - PADDING) / CELL_WIDTH);
            this.row = BOARD_SIZE - 1 - (int) Math.round((point.getY() - PADDING) / CELL_WIDTH);
        }

        public String toLabel() {
            char colChar = (char) (col + 'A');
            if (colChar >= 'I') colChar++;

            return "" + colChar + (row + 1);
        }

        public Point2D toPoint() {
            double x = PADDING + col * CELL_WIDTH;
            double y = FULL_WIDTH - (PADDING + row * CELL_WIDTH);
            return new Point2D(x, y);
        }
    }
}
