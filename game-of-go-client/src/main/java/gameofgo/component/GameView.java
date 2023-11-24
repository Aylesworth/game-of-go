package gameofgo.component;

import gameofgo.common.Message;
import gameofgo.service.SocketService;
import javafx.beans.property.SimpleStringProperty;
import javafx.geometry.Insets;
import javafx.geometry.Point2D;
import javafx.geometry.Pos;
import javafx.scene.canvas.Canvas;
import javafx.scene.canvas.GraphicsContext;
import javafx.scene.control.*;
import javafx.scene.layout.BorderPane;
import javafx.scene.layout.VBox;
import javafx.scene.paint.Color;

public class GameView extends BorderPane {
    private static final Color BACKGROUND = Color.rgb(215, 186, 137);
    private static final Color LINE_COLOR = Color.BROWN;
    private static final double FULL_WIDTH = 720;
    private static final double MARGIN = 40;

    private final int BOARD_SIZE;
    private final double CELL_WIDTH;
    private final double LINE_WIDTH;
    private final double STONE_RADIUS;
    private final int MY_COLOR;
    private boolean myTurn;
    private final SocketService socketService = SocketService.getInstance();
    private String lastPosition;
    private int lastColor;
    private String selectedPosition;

    private Canvas gameBoard;
    private GraphicsContext gc;
    private TableView<Move> tblLog;

    public GameView(int boardSize, int color) {
        this.BOARD_SIZE = boardSize;
        this.CELL_WIDTH = (FULL_WIDTH - 2 * MARGIN) / (BOARD_SIZE - 1);
        this.LINE_WIDTH = 0.05 * CELL_WIDTH;
        this.STONE_RADIUS = 0.45 * CELL_WIDTH;
        this.MY_COLOR = color;
        this.myTurn = color == 1;
        setupGameBoard();
        setUpInformationPane();
    }

    private void setupGameBoard() {
        gameBoard = new Canvas(FULL_WIDTH, FULL_WIDTH);
        gc = gameBoard.getGraphicsContext2D();

        gc.setFill(BACKGROUND);
        gc.fillRect(0, 0, FULL_WIDTH, FULL_WIDTH);

        gc.setStroke(LINE_COLOR);
        gc.setLineWidth(LINE_WIDTH);

        for (int i = 0; i < BOARD_SIZE; i++) {
            gc.strokeLine(MARGIN + i * CELL_WIDTH, MARGIN, MARGIN + i * CELL_WIDTH, FULL_WIDTH - MARGIN);
            gc.strokeLine(MARGIN, MARGIN + i * CELL_WIDTH, FULL_WIDTH - MARGIN, MARGIN + i * CELL_WIDTH);
        }

        gameBoard.setOnMouseClicked(event -> {
            if (!myTurn)
                return;

            double x = event.getX();
            double y = event.getY();

            if (x < MARGIN || x > FULL_WIDTH - MARGIN || y < MARGIN || y > FULL_WIDTH - MARGIN)
                return;

            selectedPosition = coordinatesToString(new Point2D(x, y));

            socketService.send(new Message("MOVE", "" + MY_COLOR + '\n' + selectedPosition + '\n'));
        });

        Button btnPass = new Button("Pass");
        btnPass.setDisable(!myTurn);
        btnPass.setOnMouseClicked(event -> {
            socketService.send(new Message("MOVE", "" + MY_COLOR + '\n' + "PA" + '\n'));
            myTurn = false;
            btnPass.setDisable(true);

            tblLog.getItems().add(new Move(MY_COLOR, "PA"));
        });

        socketService.on("MOVERR", message -> {
        });

        socketService.on("MOVE", message -> {
            String[] params = message.payload().split("\n");
            int color = Integer.parseInt(params[0]);
            String coords = params[1];

            tblLog.getItems().add(new Move(color, coords));

            if (!coords.equals("PA")) {
                play(coords, color);

                if (params.length > 2) {
                    String[] captured = params[2].split(" ");
                    for (String cap : captured) {
                        removeStone(cap);
                    }
                }

                myTurn = !myTurn;
                btnPass.setDisable(!myTurn);
            } else {
                myTurn = true;
                btnPass.setDisable(false);

                Alert alert = new Alert(Alert.AlertType.INFORMATION);
                alert.setHeaderText("Opponent passed");
                alert.getButtonTypes().setAll(ButtonType.OK);
                alert.showAndWait();
            }
        });

        socketService.on("RESULT", message -> {
            myTurn = false;

            String[] scores = message.payload().split(" ");
            double blackScore = Float.parseFloat(scores[0]);
            double whiteScore = Float.parseFloat(scores[1]);

            String result = MY_COLOR == 1 ? (blackScore > whiteScore ? "won" : "lost") : (whiteScore > blackScore ? "won" : "lost");

            Alert alert = new Alert(Alert.AlertType.INFORMATION);
            alert.setHeaderText("BLACK has %.0f points\nWHITE has %.1f points\nYou %s!".formatted(blackScore, whiteScore, result));
            alert.getButtonTypes().setAll(ButtonType.OK);
            alert.showAndWait();

            MainWindow.getInstance().setCenter(new HomeView());
        });

        VBox container = new VBox(gameBoard, btnPass);
        container.setAlignment(Pos.CENTER);
        container.setSpacing(15);

        setLeft(container);
    }

    private void setUpInformationPane() {
        TableColumn<Move, String> playerCol = new TableColumn<>("Player");
        playerCol.setCellValueFactory(cellData ->
                new SimpleStringProperty(cellData.getValue().getColor() == 1 ? "BLACK" : "WHITE"));

        TableColumn<Move, String> coordsCol = new TableColumn<>("Move");
        coordsCol.setCellValueFactory(cellData -> {
            Move move = cellData.getValue();
            String action = move.getCoords().equals("PA") ? "passes" : "plays " + move.getCoords();
            return new SimpleStringProperty(action);
        });

        tblLog = new TableView<>();
        tblLog.setMaxWidth(300);
        tblLog.setColumnResizePolicy(TableView.CONSTRAINED_RESIZE_POLICY);
        tblLog.getColumns().setAll(playerCol, coordsCol);

        VBox pane = new VBox();
        pane.setAlignment(Pos.CENTER);
        pane.setPadding(new Insets(0, 10, 0, 10));
        pane.setSpacing(20);
        pane.setMinWidth(500);
        pane.getChildren().addAll(tblLog);

        setRight(pane);
    }

    private void play(String coords, int color) {
        putStone(coords, color, true);
        if (lastPosition != null)
            putStone(lastPosition, lastColor, false);
        lastPosition = coords;
        lastColor = color;
    }

    private void putStone(String coords, int color, boolean withMarker) {
        Point2D coordinates = stringToCoordinates(coords);

        gc.setFill(color == 1 ? Color.BLACK : Color.WHITE);
        gc.fillOval(coordinates.getX() - STONE_RADIUS, coordinates.getY() - STONE_RADIUS, 2 * STONE_RADIUS, 2 * STONE_RADIUS);

        if (withMarker) {
            gc.setStroke(color == 1 ? Color.WHITE : Color.BLACK);
            gc.setLineWidth(3);
            gc.strokeOval(coordinates.getX() - STONE_RADIUS / 2, coordinates.getY() - STONE_RADIUS / 2, STONE_RADIUS, STONE_RADIUS);
        }
    }

    private void removeStone(String coords) {
        Point2D coordinates = stringToCoordinates(coords);
        double x = coordinates.getX();
        double y = coordinates.getY();

        gc.setFill(BACKGROUND);
        gc.fillRect(x - STONE_RADIUS, y - STONE_RADIUS, 2 * STONE_RADIUS, 2 * STONE_RADIUS);

        gc.setStroke(LINE_COLOR);
        gc.setLineWidth(LINE_WIDTH);
        if (x > MARGIN) gc.strokeLine(x - STONE_RADIUS, y, x, y);
        if (x < FULL_WIDTH - MARGIN) gc.strokeLine(x, y, x + STONE_RADIUS, y);
        if (y > MARGIN) gc.strokeLine(x, y - STONE_RADIUS, x, y);
        if (y < FULL_WIDTH - MARGIN) gc.strokeLine(x, y, x, y + STONE_RADIUS);
    }

    private Point2D stringToCoordinates(String coords) {
        int colChar = coords.charAt(0);
        if (colChar >= 'J') colChar--;

        int col = colChar - 'A';
        int row = Integer.parseInt(coords.substring(1)) - 1;

        double x = MARGIN + col * CELL_WIDTH;
        double y = FULL_WIDTH - (MARGIN + row * CELL_WIDTH);
        return new Point2D(x, y);
    }

    private String coordinatesToString(Point2D coordinates) {
        int col = (int) Math.round((coordinates.getX() - MARGIN) / CELL_WIDTH);
        int row = BOARD_SIZE - 1 - (int) Math.round((coordinates.getY() - MARGIN) / CELL_WIDTH);

        char colChar = (char) (col + 'A');
        if (colChar >= 'I') colChar++;

        return "" + colChar + (row + 1);
    }

    private class Move {
        private int color;
        private String coords;

        public Move(int color, String coords) {
            this.color = color;
            this.coords = coords;
        }

        public int getColor() {
            return color;
        }

        public String getCoords() {
            return coords;
        }
    }
}
