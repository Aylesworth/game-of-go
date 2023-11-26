package gameofgo.component;

import gameofgo.common.Configs;
import gameofgo.common.Message;
import gameofgo.service.SocketService;
import javafx.beans.property.SimpleStringProperty;
import javafx.geometry.Insets;
import javafx.geometry.Orientation;
import javafx.geometry.Point2D;
import javafx.geometry.Pos;
import javafx.scene.canvas.Canvas;
import javafx.scene.canvas.GraphicsContext;
import javafx.scene.control.*;
import javafx.scene.image.WritableImage;
import javafx.scene.layout.Background;
import javafx.scene.layout.BorderPane;
import javafx.scene.layout.FlowPane;
import javafx.scene.layout.VBox;
import javafx.scene.paint.Color;
import javafx.scene.text.TextAlignment;

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
    private Label lblBlackScore;
    private Label lblWhiteScore;
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

        Button btnSubmitMove = new Button("Submit move");
        Button btnPass = new Button("Pass");

        gameBoard.setOnMouseClicked(event -> {
            if (!myTurn)
                return;

            double x = event.getX();
            double y = event.getY();

            if (x < MARGIN || x > FULL_WIDTH - MARGIN || y < MARGIN || y > FULL_WIDTH - MARGIN)
                return;

            String position = coordinatesToString(x, y);
            if (position.equals(selectedPosition)) {
                removeStone(selectedPosition);
                selectedPosition = null;
                btnSubmitMove.setDisable(true);
                return;
            }

            x = Math.round((x - MARGIN) / CELL_WIDTH) * CELL_WIDTH + MARGIN;
            y = Math.round((y - MARGIN) / CELL_WIDTH) * CELL_WIDTH + MARGIN;

            WritableImage writableImage = new WritableImage((int) gameBoard.getWidth(), (int) gameBoard.getHeight());
            gameBoard.snapshot(null, writableImage);
            if (!writableImage.getPixelReader().getColor((int) x, (int) y).equals(LINE_COLOR)) {
                return;
            }

            if (selectedPosition != null)
                removeStone(selectedPosition);

            selectedPosition = position;

            putStone(selectedPosition, MY_COLOR, true, true);

            btnSubmitMove.setDisable(false);
        });

        btnSubmitMove.setDisable(true);
        btnSubmitMove.setOnAction(event -> {
            socketService.send(new Message("MOVE", "" + MY_COLOR + '\n' + selectedPosition + '\n'));
        });

        btnPass.setDisable(!myTurn);
        btnPass.setOnAction(event -> {
            if (selectedPosition != null)
                removeStone(selectedPosition);

            socketService.send(new Message("MOVE", "" + MY_COLOR + '\n' + "PA" + '\n'));
            myTurn = false;
            btnSubmitMove.setDisable(true);
            btnPass.setDisable(true);

            tblLog.getItems().add(new Move(MY_COLOR, "PA"));
            tblLog.scrollTo(tblLog.getItems().size() - 1);
        });

        VBox container = new VBox(gameBoard, btnSubmitMove, btnPass);
        container.setAlignment(Pos.CENTER);
        container.setSpacing(15);

        setLeft(container);

        socketService.on("MOVERR", message -> {
            removeStone(selectedPosition);
            selectedPosition = null;
        });

        socketService.on("MOVE", message -> {
            String[] params = message.payload().split("\n");
            int color = Integer.parseInt(params[0]);
            String coords = params[1];

            tblLog.getItems().add(new Move(color, coords));
            tblLog.scrollTo(tblLog.getItems().size() - 1);

            if (!coords.equals("PA")) {
                play(coords, color);

                if (params.length > 2) {
                    String[] captured = params[2].split(" ");
                    for (String cap : captured) {
                        removeStone(cap);
                    }
                    if (color == 1) {
                        lblBlackScore.setText((Integer.parseInt(lblBlackScore.getText()) + captured.length) + "");
                    } else {
                        lblWhiteScore.setText((Integer.parseInt(lblWhiteScore.getText()) + captured.length) + "");
                    }
                }

                myTurn = !myTurn;
                btnSubmitMove.setDisable(true);
                btnPass.setDisable(!myTurn);
            } else {
                myTurn = true;
                btnSubmitMove.setDisable(false);
                btnPass.setDisable(false);

                Alert alert = new Alert(Alert.AlertType.INFORMATION);
                alert.setHeaderText("Opponent passed");
                alert.getButtonTypes().setAll(ButtonType.OK);
                alert.showAndWait();
            }

            selectedPosition = null;
        });

        socketService.on("RESULT", message -> {
            myTurn = false;
            btnSubmitMove.setDisable(true);
            btnPass.setDisable(true);

            String[] params = message.payload().split("\n");
            String[] scores = params[0].split(" ");
            double blackScore = Float.parseFloat(scores[0]);
            double whiteScore = Float.parseFloat(scores[1]);

            if (params[1].length() > 1) {
                String[] blackTerritory = params[1].split(" ");
                for (String coords : blackTerritory) drawTerritory(coords, 1);
            }

            if (params[2].length() > 1) {
                String[] whiteTerritory = params[2].split(" ");
                for (String coords : whiteTerritory) drawTerritory(coords, 2);
            }

            String result = MY_COLOR == 1 ? (blackScore > whiteScore ? "won" : "lost") : (whiteScore > blackScore ? "won" : "lost");

            Alert alert = new Alert(Alert.AlertType.INFORMATION);
            alert.setHeaderText("BLACK has %.0f points\nWHITE has %.1f points\nYou %s!".formatted(blackScore, whiteScore, result));
            alert.getButtonTypes().setAll(ButtonType.OK);
            alert.showAndWait();

            MainWindow.getInstance().setCenter(new HomeView());
        });
    }

    private void setUpInformationPane() {
        Label lblTitleBlackScore = new Label("BLACK");
        lblTitleBlackScore.setMinWidth(150);
        lblTitleBlackScore.setMinHeight(50);
        lblTitleBlackScore.setBackground(Background.fill(Color.BLACK));
        lblTitleBlackScore.setTextFill(Color.WHITE);
        lblTitleBlackScore.setFont(Configs.primaryFont(16));
        lblTitleBlackScore.setTextAlignment(TextAlignment.CENTER);

        Label lblTitleWhiteScore = new Label("WHITE (+6.5)");
        lblTitleWhiteScore.setMinWidth(150);
        lblTitleWhiteScore.setMinHeight(50);
        lblTitleWhiteScore.setBackground(Background.fill(Color.WHITE));
        lblTitleWhiteScore.setFont(Configs.primaryFont(16));
        lblTitleWhiteScore.setTextAlignment(TextAlignment.CENTER);

        lblBlackScore = new Label("0");
        lblBlackScore.setMinWidth(150);
        lblBlackScore.setMinHeight(50);
        lblBlackScore.setBackground(Background.fill(Color.WHITE));
        lblBlackScore.setFont(Configs.primaryFont(16));
        lblBlackScore.setTextAlignment(TextAlignment.CENTER);

        lblWhiteScore = new Label("0");
        lblWhiteScore.setMinWidth(150);
        lblWhiteScore.setMinHeight(50);
        lblWhiteScore.setBackground(Background.fill(Color.WHITE));
        lblWhiteScore.setFont(Configs.primaryFont(16));
        lblWhiteScore.setTextAlignment(TextAlignment.CENTER);

        FlowPane scorePane = new FlowPane(Orientation.HORIZONTAL);
        scorePane.setMaxWidth(300);
        scorePane.getChildren().addAll(lblTitleBlackScore, lblTitleWhiteScore, lblBlackScore, lblWhiteScore);

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
        pane.getChildren().addAll(scorePane, tblLog);

        setRight(pane);
    }

    private void play(String coords, int color) {
        putStone(coords, color, true, false);
        if (lastPosition != null)
            putStone(lastPosition, lastColor, false, false);
        lastPosition = coords;
        lastColor = color;
    }

    private void putStone(String coords, int color, boolean withMarker, boolean faded) {
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

    private void drawTerritory(String coords, int color) {
        Point2D coordinates = stringToCoordinates(coords);
        double x = coordinates.getX();
        double y = coordinates.getY();

        gc.setFill(color == 1 ? Color.BLACK : Color.WHITE);
        gc.fillRect(x - CELL_WIDTH / 10, y - CELL_WIDTH / 10, CELL_WIDTH / 5, CELL_WIDTH / 5);
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

    private String coordinatesToString(double x, double y) {
        int col = (int) Math.round((x - MARGIN) / CELL_WIDTH);
        int row = BOARD_SIZE - 1 - (int) Math.round((y - MARGIN) / CELL_WIDTH);

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
