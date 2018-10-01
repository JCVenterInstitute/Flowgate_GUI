package flowgate

import grails.plugin.springsecurity.authentication.dao.NullSaltSource
import grails.plugin.springsecurity.ui.CommandObject
import grails.plugin.springsecurity.ui.RegistrationCode
import grails.plugin.springsecurity.SpringSecurityUtils

class RegisterController extends grails.plugin.springsecurity.ui.RegisterController {

  def adminEmail = grailsApplication.config.getProperty('adminEmailAddr', String)
  def awaitingMessage = "Your registration is awaiting admin approval! You will get the confirmation email soon."

  def register(RegisterCommand registerCommand) {

    println "in register registerCommand"

    if (!request.post) {
      return [registerCommand: new RegisterCommand()]
    }

    if (registerCommand.hasErrors()) {
      return [registerCommand: registerCommand]
    }

    /* */
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
    /* */

    sendAlphaAccessRequest registerCommand

    [emailSent: true, registerCommand: registerCommand]
  }

    def sendAlphaAccessRequest(RegisterCommand registerCommand) {
        def body = "There is a new alpha release access request <br/><br/>" +
                "<b>Name:</b> " + registerCommand.username + "<br/>" +
                "<b>Email:</b> " + registerCommand.email + "<br/>" +
                "<b>Comments:</b> " + ((registerCommand.reason == null) ? "" : registerCommand.reason) + "<br/>"
        sendMail {
            to "${adminEmail}"
            subject "New alpha release access request"
            html body
        }

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
      to "${adminEmail}"
      subject "New user registration for approval"
      html body
    }
    redirect uri: registerPostRegisterUrl ?: successHandlerDefaultTargetUrl
  }



  def forgotPassword(ForgotPasswordCommand forgotPasswordCommand) {

    if (!request.post) {
      return [forgotPasswordCommand: new ForgotPasswordCommand()]
    }

    if (forgotPasswordCommand.hasErrors()) {
      return [forgotPasswordCommand: forgotPasswordCommand]
    }
    String usernameFieldName = SpringSecurityUtils.securityConfig.userLookup.usernamePropertyName
//    User user = User.findByUsername(forgotPasswordCommand.username) //does not work!!!
//    def user = findUserByUsername(forgotPasswordCommand.username)
//    def user = lookupUserClass().findWhere((usernameFieldName): forgotPasswordCommand.username)
    def user = User.findWhere((usernameFieldName): forgotPasswordCommand.username)

    if (!user) {
      forgotPasswordCommand.errors.rejectValue 'username',
          'spring.security.ui.forgotPassword.user.notFound'
      return [forgotPasswordCommand: forgotPasswordCommand]
    }

    String email = uiPropertiesStrategy.getProperty(user, 'email')
    if (!email) {
      forgotPasswordCommand.errors.rejectValue 'username',
          'spring.security.ui.forgotPassword.noEmail'
      return [forgotPasswordCommand: forgotPasswordCommand]
    }

    RegistrationCode registrationCode = uiRegistrationCodeStrategy.sendForgotPasswordMail(
        forgotPasswordCommand.username, email) { String registrationCodeToken ->

      String url = generateLink('resetPassword', [t: registrationCodeToken])
      String body = forgotPasswordEmailBody
      if (body.contains('$')) {
        body = evaluate(body, [user: user, url: url])
      }

      body
    }
    [emailSent: true, forgotPasswordCommand: forgotPasswordCommand]
  }


  void afterPropertiesSet() {
    //super.afterPropertiesSet()

    println "username Property name = ${usernamePropertyName}"

    usernamePropertyName = 'username'

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

class ForgotPasswordCommand implements CommandObject {
  String username
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

      /* * /
      if (User.findWhere((usernamePropertyName): value)) {
        return 'registerCommand.username.unique'
      }
      / * */
    }
    email email: true
    password  nullable: true
    password2 nullable: true
    affiliation nullable: true
    reason nullable: true
  }
}