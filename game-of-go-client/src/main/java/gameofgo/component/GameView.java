package gameofgo.component;

import gameofgo.common.Configs;
import gameofgo.common.Message;
import gameofgo.service.SocketService;
import javafx.beans.property.SimpleStringProperty;
import javafx.geometry.Insets;
import javafx.geometry.Orientation;
import javafx.geometry.Pos;
import javafx.scene.control.*;
import javafx.scene.image.WritableImage;
import javafx.scene.layout.*;
import javafx.scene.paint.Color;
import javafx.scene.text.TextAlignment;

import java.util.Optional;

public class GameView extends BorderPane {
    private final SocketService socketService = SocketService.getInstance();
    private final int BOARD_SIZE;
    private final int MY_COLOR;
    private boolean myTurn;
    private boolean gameFinished;
    private String lastPosition;
    private int lastColor;
    private String selectedPosition;

    private GameCanvas gameBoard;
    private Label lblPrompt;
    private Label lblBlackScore;
    private Label lblWhiteScore;
    private TableView<Move> tblLog;

    public GameView(int boardSize, int color) {
        this.BOARD_SIZE = boardSize;
        this.MY_COLOR = color;
        this.myTurn = color == 1;
        this.gameFinished = false;
        setupGameBoard();
        setUpInformationPane();
    }

    private void setupGameBoard() {
        gameBoard = new GameCanvas(BOARD_SIZE);

        Button btnLeave = new Button("Leave");
        btnLeave.setOnAction(event -> requestQuitGame());
        HBox leaveBox = new HBox(btnLeave);

        lblPrompt = new Label("YOU ARE " + (MY_COLOR == 1 ? "BLACK" : "WHITE") + ". BLACK'S TURN");
        lblPrompt.setMinHeight(50);
        lblPrompt.setFont(Configs.primaryFont(20));
        Button btnSubmitMove = new Button("Submit move");
        Button btnPass = new Button("Pass");
        Button btnResign = new Button("Resign");

        gameBoard.setOnMouseClicked(event -> {
            if (!myTurn)
                return;

            double x = event.getX();
            double y = event.getY();

            final double MARGIN = GameCanvas.MARGIN;
            final double FULL_WIDTH = GameCanvas.FULL_WIDTH;
            final double CELL_WIDTH = (FULL_WIDTH - 2 * MARGIN) / (BOARD_SIZE - 1);

            if (x < MARGIN) x = MARGIN;
            if (x > FULL_WIDTH - MARGIN) x = FULL_WIDTH - MARGIN;
            if (y < MARGIN) y = MARGIN;
            if (y > FULL_WIDTH - MARGIN) y = FULL_WIDTH - MARGIN;

            String position = gameBoard.coordinatesToString(x, y);
            if (position.equals(selectedPosition)) {
//                removeStone(selectedPosition);
//                selectedPosition = null;
//                btnSubmitMove.setDisable(true);
                socketService.send(new Message("MOVE", "" + MY_COLOR + '\n' + selectedPosition + '\n'));
                return;
            }

            x = Math.round((x - MARGIN) / CELL_WIDTH) * CELL_WIDTH + MARGIN;
            y = Math.round((y - MARGIN) / CELL_WIDTH) * CELL_WIDTH + MARGIN;

            WritableImage writableImage = new WritableImage((int) gameBoard.getWidth(), (int) gameBoard.getHeight());
            gameBoard.snapshot(null, writableImage);
            if (!writableImage.getPixelReader().getColor((int) x, (int) y).equals(GameCanvas.LINE_COLOR)) {
                return;
            }

            if (selectedPosition != null) {
                gameBoard.remove(selectedPosition);
            }

            selectedPosition = position;

            gameBoard.drawStone(selectedPosition, MY_COLOR, false, true);

            btnSubmitMove.setDisable(false);
        });

        btnSubmitMove.setDisable(true);
        btnSubmitMove.setOnAction(event -> {
            socketService.send(new Message("MOVE", "" + MY_COLOR + '\n' + selectedPosition + '\n'));
        });

        btnPass.setDisable(!myTurn);
        btnPass.setOnAction(event -> {
            Alert alert = new Alert(Alert.AlertType.INFORMATION);
            alert.setHeaderText("Are you sure you want to pass?");
            alert.getButtonTypes().setAll(ButtonType.YES, ButtonType.NO);
            Optional<ButtonType> answer = alert.showAndWait();

            if (answer.isEmpty() || !answer.get().equals(ButtonType.YES))
                return;

            if (selectedPosition != null)
                gameBoard.remove(selectedPosition);

            if (lastPosition != null) {
                gameBoard.drawStone(lastPosition, lastColor, false, false);
                lastPosition = null;
                lastColor = 0;
            }

            socketService.send(new Message("MOVE", MY_COLOR + "\nPA\n"));
            myTurn = false;
            btnSubmitMove.setDisable(true);
            btnPass.setDisable(true);
            lblPrompt.setText(MY_COLOR == 1 ? "BLACK PASSES. WHITE'S TURN" : "WHITE PASSES. BLACK'S TURN");

            tblLog.getItems().add(new Move(MY_COLOR, "PA"));
            tblLog.scrollTo(tblLog.getItems().size() - 1);
        });

        btnResign.setOnAction(event -> {
            Alert alert = new Alert(Alert.AlertType.INFORMATION);
            alert.setHeaderText("Are you sure you want to resign?");
            alert.getButtonTypes().setAll(ButtonType.YES, ButtonType.NO);
            Optional<ButtonType> answer = alert.showAndWait();
            if (answer.isEmpty() || !answer.get().equals(ButtonType.YES))
                return;

            if (selectedPosition != null)
                gameBoard.remove(selectedPosition);

            if (lastPosition != null) {
                gameBoard.drawStone(lastPosition, lastColor, false, false);
                lastPosition = null;
                lastColor = 0;
            }

            socketService.send(new Message("INTRPT", MY_COLOR + "\nRESIGN\n"));
            myTurn = false;
            btnSubmitMove.setDisable(true);
            btnPass.setDisable(true);
            lblPrompt.setText(MY_COLOR == 1 ? "BLACK RESIGNS. WHITE WINS!" : "WHITE RESIGNS. BLACK WINS!");

            tblLog.getItems().add(new Move(MY_COLOR, "RS"));
            tblLog.scrollTo(tblLog.getItems().size() - 1);
        });

        HBox buttonBox = new HBox(btnSubmitMove, btnPass, btnResign);
        buttonBox.setAlignment(Pos.CENTER);
        buttonBox.setSpacing(20);

        VBox container = new VBox(leaveBox, lblPrompt, gameBoard, buttonBox);
        container.setAlignment(Pos.TOP_CENTER);
        container.setSpacing(15);

        setLeft(container);

        socketService.on("MOVERR", message -> {
            gameBoard.remove(selectedPosition);
            selectedPosition = null;
            lblPrompt.setText("INVALID MOVE!");
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
                        gameBoard.remove(cap);
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
                lblPrompt.setText(color == 1 ? "WHITE'S TURN" : "BLACK'S TURN");
            } else {
                if (lastPosition != null) {
                    gameBoard.drawStone(lastPosition, lastColor, false, false);
                    lastPosition = null;
                    lastColor = 0;
                }
                myTurn = true;
                btnSubmitMove.setDisable(false);
                btnPass.setDisable(false);
                lblPrompt.setText(color == 1 ? "BLACK PASSES. WHITE'S TURN" : "WHITE PASSES. BLACK'S TURN");
            }

            selectedPosition = null;
        });

        socketService.on("INTRPT", message -> {
            String[] params = message.payload().split("\n");
            int color = Integer.parseInt(params[0]);
            if (params[1].equals("RESIGN")) {
                myTurn = false;
                btnSubmitMove.setDisable(true);
                btnPass.setDisable(true);
                btnResign.setDisable(true);
                lblPrompt.setText(color == 1 ? "BLACK RESIGNS. WHITE WINS!" : "WHITE RESIGNS. BLACK WINS!");
                tblLog.getItems().add(new Move(MY_COLOR, "RS"));
                tblLog.scrollTo(tblLog.getItems().size() - 1);
            }
        });

        socketService.on("RESULT", message -> {
            System.out.println(lastPosition);
            if (lastPosition != null) {
                gameBoard.drawStone(lastPosition, lastColor, false, false);
                lastPosition = null;
                lastColor = 0;
            }

            gameFinished = true;
            myTurn = false;
            btnSubmitMove.setDisable(true);
            btnPass.setDisable(true);
            btnResign.setDisable(true);

            String[] params = message.payload().split("\n");
            String[] scores = params[0].split(" ");
            double blackScore = Float.parseFloat(scores[0]);
            double whiteScore = Float.parseFloat(scores[1]);
            String winner = blackScore > whiteScore ? "BLACK" : "WHITE";

            if (params.length > 1 && params[1].length() > 1) {
                String[] blackTerritory = params[1].split(" ");
                gameBoard.drawTerritory(1, blackTerritory);
            }

            if (params.length > 2 && params[2].length() > 1) {
                String[] whiteTerritory = params[2].split(" ");
                gameBoard.drawTerritory(2, whiteTerritory);
            }

            if (blackScore >= 0 && whiteScore >= 0)
                lblPrompt.setText("BLACK %.1f : %.1f WHITE. ".formatted(blackScore, whiteScore) + winner + " WINS!");
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
            String action = switch (move.getCoords()) {
                case "PA" -> "passes";
                case "RS" -> "resigns";
                default -> "plays " + move.getCoords();
            };
            return new SimpleStringProperty(action);
        });

        tblLog = new TableView<>();
        tblLog.setMaxWidth(300);
        tblLog.setColumnResizePolicy(TableView.CONSTRAINED_RESIZE_POLICY);
        tblLog.getColumns().setAll(playerCol, coordsCol);

        Label lblCaptured = new Label("CAPTURED");
        lblCaptured.setFont(Configs.primaryFont(20));
        Label lblLog = new Label("GAME LOG");
        lblLog.setFont(Configs.primaryFont(20));

        VBox pane = new VBox();
        pane.setAlignment(Pos.CENTER);
        pane.setPadding(new Insets(0, 10, 0, 10));
        pane.setSpacing(20);
        pane.setMinWidth(500);
        pane.getChildren().addAll(lblCaptured, scorePane, lblLog, tblLog);

        setRight(pane);
    }

    public boolean requestQuitGame() {
        if (gameFinished) {
            socketService.send(new Message("RESACK", ""));
            MainWindow.getInstance().previous();
            return true;
        }

        Alert alert = new Alert(Alert.AlertType.INFORMATION);
        alert.setHeaderText("Do you really want to quit this game?\nThe game will be counted as your defeat.");
        alert.getButtonTypes().setAll(ButtonType.YES, ButtonType.NO);
        Optional<ButtonType> answer = alert.showAndWait();

        if (answer.isPresent() && answer.get().equals(ButtonType.YES)) {
            socketService.send(new Message("INTRPT", MY_COLOR + "\nRESIGN\n"), 0, 10);
            socketService.send(new Message("RESACK", ""), 0, 10);
            MainWindow.getInstance().previous();
            return true;
        }
        return false;
    }

    private void play(String coords, int color) {
        gameBoard.drawStone(coords, color, true, false);
        if (lastPosition != null)
            gameBoard.drawStone(lastPosition, lastColor, false, false);
        lastPosition = coords;
        lastColor = color;
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
