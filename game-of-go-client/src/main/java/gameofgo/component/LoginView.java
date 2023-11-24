package gameofgo.component;

import gameofgo.common.Configs;
import gameofgo.common.Message;
import gameofgo.common.SessionStorage;
import gameofgo.service.SocketService;
import javafx.event.ActionEvent;
import javafx.geometry.Insets;
import javafx.geometry.Pos;
import javafx.scene.control.Button;
import javafx.scene.control.Label;
import javafx.scene.control.PasswordField;
import javafx.scene.control.TextField;
import javafx.scene.layout.Background;
import javafx.scene.layout.VBox;
import javafx.scene.paint.Color;

public class LoginView extends VBox {
    private SocketService socketService = SocketService.getInstance();
    private Label lblTitle;
    private Label lblUsername;
    private Label lblPassword;
    private TextField txtUsername;
    private TextField txtPassword;
    private Label lblMessage;
    private Button btnSignIn;
    private Label lblSignUp;

    public LoginView() {
        double fieldWidth = 200;

        lblTitle = new Label("LOGIN");
        lblTitle.setFont(Configs.primaryFont(20));

        lblUsername = new Label("Username");
        lblUsername.setFont(Configs.primaryFont(14));
        lblUsername.setMinWidth(fieldWidth);

        lblPassword = new Label("Password");
        lblPassword.setFont(Configs.primaryFont(14));
        lblPassword.setMinWidth(fieldWidth);

        txtUsername = new TextField();
        txtUsername.setPromptText("Username");
        txtUsername.setMaxWidth(fieldWidth);

        txtPassword = new PasswordField();
        txtPassword.setPromptText("Password");
        txtPassword.setMaxWidth(fieldWidth);

        lblMessage = new Label();
        lblMessage.setTextFill(Color.RED);
        lblMessage.setWrapText(true);

        btnSignIn = new Button("Login");

        lblSignUp = new Label("No account? Register");
        lblSignUp.setTextFill(Color.BLUE);
        lblSignUp.setOnMouseEntered(event -> lblSignUp.setUnderline(true));
        lblSignUp.setOnMouseExited(event -> lblSignUp.setUnderline(false));
        lblSignUp.setOnMouseClicked(event -> MainWindow.getInstance().setCenter(new RegisterView()));

        setMaxWidth(300);
        setAlignment(Pos.CENTER);
        getChildren().addAll(
                lblTitle,
                lblUsername,
                txtUsername,
                lblPassword,
                txtPassword,
                lblMessage,
                btnSignIn,
                lblSignUp
        );

        Insets defaultMargin = new Insets(0, 0, 10, 0);
        setMargin(lblTitle, defaultMargin);
        setMargin(txtUsername, defaultMargin);
        setMargin(txtPassword, defaultMargin);
        setMargin(btnSignIn, defaultMargin);
        setMargin(lblMessage, new Insets(0, 0, 5, 0));

        btnSignIn.setOnAction(this::onSubmit);
        txtUsername.setOnAction(this::onSubmit);
        txtPassword.setOnAction(this::onSubmit);

        socketService.on("OK", message -> {
            SessionStorage.setItem("username", txtUsername.getText());
            MainWindow.getInstance().setCenter(new HomeView());
        });
        socketService.on("ERROR", message -> lblMessage.setText(message.payload()));
    }

    private void onSubmit(ActionEvent event) {
        try {
            validate();
        } catch (Exception e) {
            lblMessage.setText(e.getMessage());
            return;
        }

        socketService.send(new Message(
                "LOGIN",
                txtUsername.getText() + '\n' + txtPassword.getText() + '\n'
        ));
    }

    private void validate() throws Exception {
        lblMessage.setText("");
        txtUsername.setBackground(Background.fill(Color.WHITE));
        txtPassword.setBackground(Background.fill(Color.WHITE));

        if (txtUsername.getText().isEmpty()) {
            txtUsername.setBackground(Background.fill(Configs.ERROR_COLOR));
            throw new Exception("Please enter your username");
        }

        if (txtPassword.getText().isEmpty()) {
            txtPassword.setBackground(Background.fill(Configs.ERROR_COLOR));
            throw new Exception("Please enter your password");
        }
    }
}
