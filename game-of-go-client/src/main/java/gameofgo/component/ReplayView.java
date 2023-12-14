package gameofgo.component;

import gameofgo.common.Configs;
import gameofgo.common.GameReplay;
import gameofgo.service.SocketService;
import javafx.application.Platform;
import javafx.beans.property.SimpleStringProperty;
import javafx.collections.FXCollections;
import javafx.geometry.Insets;
import javafx.geometry.Orientation;
import javafx.geometry.Pos;
import javafx.scene.Node;
import javafx.scene.control.*;
import javafx.scene.layout.*;
import javafx.scene.paint.Color;
import javafx.scene.text.Font;
import javafx.scene.text.TextAlignment;

public class ReplayView extends BorderPane {
    private final SocketService socketService = SocketService.getInstance();
    private final GameReplay gameReplay;
    private final int boardSize;
    private final String[] moves;
    private final String[] prompts;
    private final String[] trackMove;
    private int index;
    private int speedMillis;
    private GameCanvas gameBoard;
    private Label lblPrompt;
    private Label lblBlackScore, lblWhiteScore;
    private TableView<String> tblLog;
    private Thread playThread;
    private Button btnPlay, btnPrev, btnNext, btnFirst, btnLast;

    public ReplayView(GameReplay gameReplay) {
        this.gameReplay = gameReplay;
        this.boardSize = gameReplay.boardSize();
        this.moves = gameReplay.log().split(" ");
        this.prompts = new String[moves.length];
        this.trackMove = new String[moves.length];
        this.index = -1;
        this.speedMillis = 1000;
        setupGameBoard();
        setUpInformationPane();
    }

    private void setupGameBoard() {
        this.gameBoard = new GameCanvas(boardSize);

        Button btnLeave = new Button("Leave");
        btnLeave.setOnAction(event -> {
            pause();
            MainWindow.getInstance().previous();
        });
        HBox leaveBox = new HBox(btnLeave);

        lblPrompt = new Label("YOU ARE %s. BLACK'S TURN".formatted(gameReplay.color() == 1 ? "BLACK" : "WHITE"));
        lblPrompt.setMinHeight(50);
        lblPrompt.setFont(Configs.primaryFont(20));

        btnPlay = new Button("▶");
        btnPrev = new Button("‹");
        btnNext = new Button("›");
        btnFirst = new Button("«");
        btnLast = new Button("»");

        btnPlay.setOnAction(event -> play());
        btnPrev.setOnAction(event -> prev());
        btnNext.setOnAction(event -> next());
        btnFirst.setOnAction(event -> first());
        btnLast.setOnAction(event -> last());

        HBox controlBox = new HBox(10, btnFirst, btnPrev, btnPlay, btnNext, btnLast);
        controlBox.setAlignment(Pos.CENTER);
        for (Node node : controlBox.getChildren()) {
            Button button = (Button) node;
            button.setFont(Font.font(16));
            button.setPrefWidth(30);
            button.setPrefHeight(30);
            button.setPadding(new Insets(2));
        }

        Label lblSpeed = new Label("Speed:");
        lblSpeed.setFont(Configs.primaryFont(13));

        ComboBox<String> cbSpeed = new ComboBox<>(FXCollections.observableArrayList(
                "x0.25",
                "x0.5",
                "x1",
                "x2",
                "x3",
                "x5",
                "x10"
        ));
        cbSpeed.getSelectionModel().select(2);
        cbSpeed.valueProperty().addListener(((observable, oldValue, newValue) -> {
            speedMillis = (int) (1 / Double.parseDouble(newValue.substring(1)) * 1000);
        }));

        HBox speedBox = new HBox(10, lblSpeed, cbSpeed);
        speedBox.setAlignment(Pos.CENTER_RIGHT);
        speedBox.setMaxWidth(200);

        StackPane stackPane = new StackPane(controlBox, speedBox);
        stackPane.setAlignment(Pos.CENTER_RIGHT);

        VBox container = new VBox(leaveBox, lblPrompt, gameBoard, stackPane);
        container.setAlignment(Pos.TOP_CENTER);
        container.setSpacing(15);

        setLeft(container);
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

        TableColumn<String, String> playerCol = new TableColumn<>("Player");
        playerCol.setCellValueFactory(cellData ->
                new SimpleStringProperty(cellData.getValue().charAt(0) - '0' == 1 ? "BLACK" : "WHITE"));

        TableColumn<String, String> coordsCol = new TableColumn<>("Move");
        coordsCol.setCellValueFactory(cellData -> {
            String param = cellData.getValue().substring(2);
            String action = switch (param) {
                case "PA" -> "passes";
                case "RS" -> "resigns";
                default -> "plays " + param;
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

    private void updateScore(int color, int difference) {
        if (color == 1) {
            lblBlackScore.setText((Integer.parseInt(lblBlackScore.getText()) + difference) + "");
        } else {
            lblWhiteScore.setText((Integer.parseInt(lblWhiteScore.getText()) + difference) + "");
        }
    }

    private void play() {
        btnPlay.setText("⏸");
        btnPlay.setOnAction(event -> pause());

        playThread = new Thread(() -> {
            try {
                while (!Thread.currentThread().isInterrupted() && next()) {
                    Thread.sleep(speedMillis);
                }
                pause();
            } catch (InterruptedException e) {
                System.out.println("Play thread stopped");
            }
        });
        playThread.start();
    }

    private void pause() {
        Platform.runLater(() -> {
            btnPlay.setText("▶");
            btnPlay.setOnAction(event -> play());
        });

        if (playThread != null)
            playThread.interrupt();
    }

    private boolean next() {
        if (++index >= moves.length) {
            index = moves.length - 1;
            return false;
        }

        String move = moves[index];
        String[] actions = move.split("/");
        Platform.runLater(() -> {
            tblLog.getItems().add(actions[0]);
            tblLog.scrollTo(tblLog.getItems().size() - 1);
        });

        for (String action : actions) {
            int color = action.charAt(0) - '0';
            char type = action.charAt(1);
            String param = action.substring(2);
            switch (type) {
                case '+' -> {
                    prompts[index] = color == 1 ? "WHITE'S TURN" : "BLACK'S TURN";
                    trackMove[index] = param;
                    String lastPosition = index - 1 >= 0 ? trackMove[index - 1] : null;
                    int lastColor = (index - 1) % 2 + 1;
                    Platform.runLater(() -> {
                        if (index >= 1 && lastPosition != null) {
                            gameBoard.drawStone(lastPosition, lastColor, false, false);
                        }

                        gameBoard.drawStone(param, color, true, false);
                        lblPrompt.setText(prompts[index]);
                    });
                }
                case '-' -> {
                    String[] targets = param.split(",");
                    Platform.runLater(() -> {
                        gameBoard.remove(targets);
                        updateScore(3 - color, targets.length);
                    });
                }
                case '=' -> {
                    String lastPosition = index - 1 >= 0 ? trackMove[index - 1] : null;
                    int lastColor = (index - 1) % 2 + 1;
                    Platform.runLater(() -> {
                        if (index >= 1 && lastPosition != null) {
                            gameBoard.drawStone(lastPosition, lastColor, false, false);
                        }
                    });
                    switch (param) {
                        case "PA" -> {
                            if (index == moves.length - 1) {
                                prompts[index] = "BLACK %.1f : %.1f WHITE. %s WINS!".formatted(
                                        gameReplay.blackScore(), gameReplay.whiteScore(),
                                        gameReplay.blackScore() > gameReplay.whiteScore() ? "BLACK" : "WHITE"
                                );
                                Platform.runLater(() -> {
                                    lblPrompt.setText(prompts[index]);
                                    gameBoard.drawTerritory(1, gameReplay.blackTerritory());
                                    gameBoard.drawTerritory(2, gameReplay.whiteTerritory());
                                });
                            } else {
                                prompts[index] = color == 1 ? "BLACK PASSES. WHITE'S TURN" : "WHITE PASSES. BLACK'S TURN";
                                Platform.runLater(() -> lblPrompt.setText(prompts[index]));
                            }
                        }
                        case "RS" -> {
                            prompts[index] = color == 1 ? "BLACK RESIGNS. WHITE WINS!" : "WHITE RESIGNS. BLACK WINS!";
                            Platform.runLater(() -> {
                                lblPrompt.setText(prompts[index]);
                                gameBoard.drawTerritory(1, gameReplay.blackTerritory());
                                gameBoard.drawTerritory(2, gameReplay.blackTerritory());
                            });
                        }
                    }
                }
            }
        }
        return true;
    }

    private boolean prev() {
        if (index < 0) return false;

        tblLog.getItems().remove(tblLog.getItems().size() - 1);
        tblLog.scrollTo(tblLog.getItems().size() - 1);

        if (index == moves.length - 1) {
            gameBoard.remove(gameReplay.blackTerritory());
            gameBoard.remove(gameReplay.whiteTerritory());
        }

        String move = moves[index];
        String[] actions = move.split("/");
        for (String action : actions) {
            int color = action.charAt(0) - '0';
            char type = action.charAt(1);
            String param = action.substring(2);
            switch (type) {
                case '+' -> {
                    gameBoard.remove(param);
                }
                case '-' -> {
                    String[] targets = param.split(",");
                    for (String target : targets)
                        gameBoard.drawStone(target, color, false, false);
                    updateScore(3 - color, -targets.length);
                }
            }
        }

        if (--index < 0) {
            lblPrompt.setText("YOU ARE %s. BLACK'S TURN".formatted(gameReplay.color() == 1 ? "BLACK" : "WHITE"));
        } else {
            lblPrompt.setText(prompts[index]);
            if (trackMove[index] != null) {
                gameBoard.drawStone(trackMove[index], index % 2 + 1, true, false);
            }
        }
        return true;
    }

    private void first() {
        pause();
        while (prev()) ;
    }

    private void last() {
        pause();
        while (next()) ;
    }
}