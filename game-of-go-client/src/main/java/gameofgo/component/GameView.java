package gameofgo.component;

import gameofgo.common.Message;
import gameofgo.service.SocketService;
import javafx.geometry.Point2D;
import javafx.scene.canvas.Canvas;
import javafx.scene.canvas.GraphicsContext;
import javafx.scene.control.Button;
import javafx.scene.layout.BorderPane;
import javafx.scene.layout.VBox;
import javafx.scene.paint.Color;

public class GameView extends BorderPane {
    private static final Color BACKGROUND = Color.rgb(245, 183, 91);
    private static final Color LINE_COLOR = Color.BROWN;
    private static final double FULL_WIDTH = 720;
    private static final double PADDING = 40;

    private final int BOARD_SIZE = 13;
    private final double CELL_WIDTH = (FULL_WIDTH - 2 * PADDING) / (BOARD_SIZE - 1);
    private final double STONE_RADIUS = CELL_WIDTH * 0.45;
    private final int MY_COLOR;
    private boolean myTurn;
    private final SocketService socketService = SocketService.getInstance();

    private Canvas gameBoard;
    private GraphicsContext gc;
    private String lastPosition;
    private int lastColor;
    private String selectedPosition;

    public GameView(int color) {
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

            selectedPosition = coordinatesToLabel(new Point2D(x, y));

            socketService.send(new Message("MOVE", "" + MY_COLOR + '\n' + selectedPosition + '\n'));
        });

        Button btnPass = new Button("Pass");
        btnPass.setDisable(!myTurn);
        btnPass.setOnMouseClicked(event -> {
            socketService.send(new Message("MOVPAS", "" + MY_COLOR + '\n'));
            myTurn = false;
            btnPass.setDisable(true);
        });

        socketService.on("MOVERR", message -> {
        });

        socketService.on("MOVE", message -> {
            String[] params = message.payload().split("\n");
            int color = Integer.parseInt(params[0]);
            String label = params[1];
            play(label, color);

            if (params.length > 2) {
                String[] captured = params[2].split(" ");
                for (String cap : captured) {
                    removeStone(cap);
                }
            }
            myTurn = !myTurn;
            btnPass.setDisable(!myTurn);
        });

        socketService.on("MOVPAS", message -> {
            myTurn = true;
            btnPass.setDisable(false);
        });

        socketService.on("RESULT", message -> {
            System.out.printf("You've got %.1f points%n", Float.parseFloat(message.payload().split("\n")[0]));
            myTurn = false;
        });

        VBox container = new VBox(gameBoard, btnPass);

        setLeft(container);
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

        gc.setFill(color == 1 ? Color.BLACK : Color.WHITE);
        gc.fillOval(coordinates.getX() - STONE_RADIUS, coordinates.getY() - STONE_RADIUS, 2 * STONE_RADIUS, 2 * STONE_RADIUS);

        if (withMarker) {
            gc.setStroke(color == 1 ? Color.WHITE : Color.BLACK);
            gc.setLineWidth(3);
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
