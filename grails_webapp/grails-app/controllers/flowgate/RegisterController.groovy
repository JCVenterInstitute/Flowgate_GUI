package flowgate

import grails.plugin.springsecurity.authentication.dao.NullSaltSource
import grails.plugin.springsecurity.ui.CommandObject
import grails.plugin.springsecurity.ui.RegistrationCode

class RegisterController extends grails.plugin.springsecurity.ui.RegisterController {

  def adminEmail = grailsApplication.config.getProperty('adminEmailAddr', String)
  def awaitingMessage = "Your registration is awaiting admin approval! You will get the confirmation email soon."

  def register(RegisterCommand registerCommand) {

    if (!request.post) {
      return [registerCommand: new RegisterCommand()]
    }

    if (registerCommand.hasErrors()) {
      return [registerCommand: registerCommand]
    }

    def user = uiPropertiesStrategy.setProperties(
        email: registerCommand.email, username: registerCommand.username, affiliation: registerCommand.affiliation,
        reason: registerCommand.reason, accountLocked: true, enabled: true, User, null)

    String salt = saltSource instanceof NullSaltSource ? null : registerCommand.username
    RegistrationCode registrationCode = uiRegistrationCodeStrategy.register(user, registerCommand.password, salt)

    if (registrationCode == null || registrationCode.hasErrors()) {
      // null means problem creating the user
      flash.error = message(code: 'spring.security.ui.register.miscError')
      return [registerCommand: registerCommand]
    }

    sendVerifyRegistrationMail registrationCode, user, registerCommand.email

    [emailSent: true, registerCommand: registerCommand]
  }

  def verifyRegistration() {

    String token = params.t

    RegistrationCode registrationCode = token ? RegistrationCode.findByToken(token) : null
    if (!registrationCode) {
      flash.error = message(code: 'spring.security.ui.register.badCode')
      redirect uri: successHandlerDefaultTargetUrl
      return
    }

    def user = uiRegistrationCodeStrategy.finishRegistration(registrationCode)

    if (!user) {
      flash.error = message(code: 'spring.security.ui.register.badCode')
      redirect uri: successHandlerDefaultTargetUrl
      return
    }

    if (user.hasErrors()) {
      // expected to be handled already by ErrorsStrategy.handleValidationErrors
      return
    }

    flash.message = message(code: 'spring.security.ui.register.complete')
    //String adminEmail = User.findByUsername('admin').email
    def body = "There is a new user waiting for your approval <br/><br/>" +
        "<b>Username:</b> " + user.username + "<br/>" +
        "<b>Email:</b> " + user.email + "<br/>" +
        "<b>Affiliation:</b> " + user.affiliation + "<br/>" +
        "<b>Reason:</b> " + ((user.reason == null) ? "" : user.reason) + "<br/>" +
        "<b>Go to users page for approval!</b> "
    sendMail {
      to adminEmail
      subject "New user registration for approval"
      html body
    }
    redirect uri: registerPostRegisterUrl ?: successHandlerDefaultTargetUrl
  }

  void afterPropertiesSet() {
    super.afterPropertiesSet()

    RegisterCommand.User = User
    RegisterCommand.usernamePropertyName = usernamePropertyName

    forgotPasswordEmailBody = conf.ui.forgotPassword.emailBody ?: ''
    registerEmailBody = conf.ui.register.emailBody ?: ''
    registerEmailFrom = conf.ui.register.emailFrom ?: ''
    registerEmailSubject = conf.ui.register.emailSubject ?: ''
    registerPostRegisterUrl = conf.ui.register.postRegisterUrl ?: ''
    registerPostResetUrl = conf.ui.register.postResetUrl ?: ''
    successHandlerDefaultTargetUrl = conf.successHandler.defaultTargetUrl ?: '/'

    passwordMaxLength = conf.ui.password.maxLength instanceof Number ? conf.ui.password.maxLength : 64
    passwordMinLength = conf.ui.password.minLength instanceof Number ? conf.ui.password.minLength : 8
    passwordValidationRegex = conf.ui.password.validationRegex ?: '^.*(?=.*\\d)(?=.*[a-zA-Z])(?=.*[!@#$%^&]).*$'
  }
}

class RegisterCommand implements CommandObject {

  protected static Class<?> User
  protected static String usernamePropertyName

  String username
  String email
  String affiliation
  String reason
  String password
  String password2

  static constraints = {
    username validator: { value, command ->
      if (!value) {
        return
      }

      if (User.findWhere((usernamePropertyName): value)) {
        return 'registerCommand.username.unique'
      }
    }
    email email: true
    password validator: RegisterController.passwordValidator
    password2 nullable: true, validator: RegisterController.password2Validator
    affiliation nullable: false
    reason nullable: true
  }
}