package gameofgo.component;

import javafx.geometry.Point2D;
import javafx.scene.canvas.Canvas;
import javafx.scene.canvas.GraphicsContext;
import javafx.scene.paint.Color;

public class GameCanvas extends Canvas {
    public static final Color BACKGROUND = Color.rgb(215, 186, 137);
    public static final Color LINE_COLOR = Color.BROWN;
    public static final double FULL_WIDTH = 720;
    public static final double MARGIN = 40;

    private final int BOARD_SIZE;
    private final double CELL_WIDTH;
    private final double LINE_WIDTH;
    private final double STONE_RADIUS;
    private GraphicsContext gc;

    public GameCanvas(int boardSize) {
        super(FULL_WIDTH, FULL_WIDTH);
        this.BOARD_SIZE = boardSize;
        this.CELL_WIDTH = (FULL_WIDTH - 2 * MARGIN) / (BOARD_SIZE - 1);
        this.LINE_WIDTH = 0.05 * CELL_WIDTH;
        this.STONE_RADIUS = 0.45 * CELL_WIDTH;
        this.gc = this.getGraphicsContext2D();
        setup();
    }

    public void setup() {
        gc.setFill(BACKGROUND);
        gc.fillRect(0, 0, FULL_WIDTH, FULL_WIDTH);

        gc.setStroke(LINE_COLOR);
        gc.setLineWidth(LINE_WIDTH);

        for (int i = 0; i < BOARD_SIZE; i++) {
            gc.strokeLine(MARGIN + i * CELL_WIDTH, MARGIN, MARGIN + i * CELL_WIDTH, FULL_WIDTH - MARGIN);
            gc.strokeLine(MARGIN, MARGIN + i * CELL_WIDTH, FULL_WIDTH - MARGIN, MARGIN + i * CELL_WIDTH);
        }
    }

    public void drawStone(String coords, int color, boolean withMarker, boolean faded) {
        Point2D coordinates = stringToCoordinates(coords);

        double opacity = faded ? 0.5 : 1;
        Color black = new Color(0, 0, 0, opacity);
        Color white = new Color(1, 1, 1, opacity);

        gc.setFill(color == 1 ? black : white);
        gc.fillOval(coordinates.getX() - STONE_RADIUS, coordinates.getY() - STONE_RADIUS, 2 * STONE_RADIUS, 2 * STONE_RADIUS);

        if (withMarker) {
            gc.setStroke(color == 1 ? white : black);
            gc.setLineWidth(3);
            gc.strokeOval(coordinates.getX() - STONE_RADIUS / 2, coordinates.getY() - STONE_RADIUS / 2, STONE_RADIUS, STONE_RADIUS);
        }
    }

    public void remove(String... coordsList) {
        gc.setFill(BACKGROUND);
        gc.setStroke(LINE_COLOR);

        for (String coords : coordsList) {
            Point2D coordinates = stringToCoordinates(coords);
            double x = coordinates.getX();
            double y = coordinates.getY();

            gc.fillRect(x - STONE_RADIUS, y - STONE_RADIUS, 2 * STONE_RADIUS, 2 * STONE_RADIUS);
            gc.setLineWidth(LINE_WIDTH);
            if (x > MARGIN) gc.strokeLine(x - STONE_RADIUS, y, x, y);
            if (x < FULL_WIDTH - MARGIN) gc.strokeLine(x, y, x + STONE_RADIUS, y);
            if (y > MARGIN) gc.strokeLine(x, y - STONE_RADIUS, x, y);
            if (y < FULL_WIDTH - MARGIN) gc.strokeLine(x, y, x, y + STONE_RADIUS);
        }
    }

    public void drawTerritory(int color, String... coordsList) {
        gc.setFill(color == 1 ? Color.BLACK : Color.WHITE);
        for (String coords : coordsList) {
            Point2D coordinates = stringToCoordinates(coords);
            double x = coordinates.getX();
            double y = coordinates.getY();
            gc.fillRect(x - CELL_WIDTH / 10, y - CELL_WIDTH / 10, CELL_WIDTH / 5, CELL_WIDTH / 5);
        }
    }

    public Point2D stringToCoordinates(String coords) {
        int colChar = coords.charAt(0);
        if (colChar >= 'J') colChar--;

        int col = colChar - 'A';
        int row = Integer.parseInt(coords.substring(1)) - 1;

        double x = MARGIN + col * CELL_WIDTH;
        double y = FULL_WIDTH - (MARGIN + row * CELL_WIDTH);
        return new Point2D(x, y);
    }

    public String coordinatesToString(double x, double y) {
        int col = (int) Math.round((x - MARGIN) / CELL_WIDTH);
        int row = BOARD_SIZE - 1 - (int) Math.round((y - MARGIN) / CELL_WIDTH);

        char colChar = (char) (col + 'A');
        if (colChar >= 'I') colChar++;

        return "" + colChar + (row + 1);
    }
}
