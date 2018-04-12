grails {
	mail {
		host = "smtp.gmail.com"
		port = 465
		username = "flowgate.noreply@gmail.com"
		password = "FlowGate123"
		props = ["mail.smtp.auth":"true",
				 "mail.smtp.socketFactory.port":"465",
				 "mail.smtp.socketFactory.class":"javax.net.ssl.SSLSocketFactory",
				 "mail.smtp.socketFactory.fallback":"false"]
	}
}

grails.resources.adhoc.includes = ['/images/**', '/css/**', '/js/**', '/plugins/**', '/fonts/**']

grails.plugin.springsecurity.ui.register.postRegisterUrl = '/'
grails.plugin.springsecurity.ui.register.defaultRoleNames = ['ROLE_NewUser','ROLE_Guest']
//grails.plugin.springsecurity.ui.gsp.layoutRegister = 'main'

//grails.plugin.springsecurity.ui.password.validationRegex='^([a-zA-Z0-9@*#_%])$'
//grails.plugin.springsecurity.ui.password.validationRegex='^([a-zA-Z0-9@*#_%])$'
//grails.plugin.springsecurity.ui.password.minLength=3
//grails.plugin.springsecurity.ui.password.maxLength=64

grails.plugin.springsecurity.rest.login.active  = true // otherwise no token

// Added by the Spring Security Core plugin:

grails.plugin.springsecurity.logout.postOnly = false
//grails.plugin.springsecurity.logout.afterLogoutUrl = "/dashboard"
grails.plugin.springsecurity.logout.afterLogoutUrl = "/"
grails.plugin.springsecurity.successHandler.defaultTargetUrl = '/project/list'
//grails.plugin.springsecurity.auth.loginFormUrl = '/login/auth'
grails.plugin.springsecurity.auth.loginFormUrl = '/index'
grails.plugin.springsecurity.userLookup.userDomainClassName = 'flowgate.User'
grails.plugin.springsecurity.userLookup.authorityJoinClassName = 'flowgate.UserRole'
grails.plugin.springsecurity.authority.className = 'flowgate.Role'
grails.plugin.springsecurity.controllerAnnotations.staticRules = [
//	'/j_spring_security_switch_user': ['ROLE_ADMIN'],
//'/j_spring_security_exit_user':   ['permitAll']
	[pattern: '/upload/**',				access: ['ROLE_ADMIN','ROLE_USER']],
	[pattern: '/user/**', 				access: ['ROLE_Admin','ROLE_Administrator']],
	[pattern: '/logout/impersonate', 	access: ['permitAll']],
	[pattern: '/dashboard',             access: ['permitAll']],
	[pattern: '/dashboard/index.gsp',   access: ['permitAll']],
	[pattern: '/login/**',              access: ['permitAll']],
	[pattern: '/login/impersonate', 	access: ['ROLE_Admin','ROLE_Administrator']],
	[pattern: '/logout/**',             access: ['permitAll']],
	[pattern: '/logout/index',          access: ['permitAll']],
	[pattern: '/logout/index.gsp',      access: ['permitAll']],
	[pattern: '/logoff/**',             access: ['permitAll']],
	[pattern: '/logoff/index',          access: ['permitAll']],
	[pattern: '/logoff/index.gsp',      access: ['permitAll']],
	[pattern: '/error',          		access: ['permitAll']],
	[pattern: '/**',          			access: ['permitAll']],
//	[pattern: '/project/list',		    access: ['permitAll']],
	[pattern: '/mgmnt/index.gsp',      	access: ['permitAll']],
	[pattern: '/register/**',      		access: ['permitAll']],
	[pattern: '/dbconsole/**',     		access: ['permitAll']],
	[pattern: '/shutdown',       		access: ['permitAll']],
	[pattern: '/assets/**',      		access: ['permitAll']],
	[pattern: '/assets/images/**', 		access: ['permitAll']],
	[pattern: '/**/fonts/**',           access: ['permitAll']],
	[pattern: '/**/js/**',       		access: ['permitAll']],
	[pattern: '/**/html/**',       		access: ['permitAll']],
	[pattern: '/**/css/**',      		access: ['permitAll']],
	[pattern: '/**/images/**',   		access: ['permitAll']],
	[pattern: '/**/favicon.ico', 		access: ['permitAll']]
]

grails.plugin.springsecurity.filterChain.chainMap = [
	[pattern: '/assets/**',      filters: 'none'],
	[pattern: '/assets/images/**',      filters: 'none'],
	[pattern: '/**/js/**',       filters: 'none'],
	[pattern: '/**/html/**',       filters: 'none'],
	[pattern: '/**/css/**',      filters: 'none'],
	[pattern: '/**/images/**',   filters: 'none'],
	[pattern: '/**/favicon.ico', filters: 'none'],
//	[pattern: '/project/list',	 filters: 'none'],
	[pattern: '/api/**', filters:'JOINED_FILTERS,-anonymousAuthenticationFilter,-exceptionTranslationFilter,-authenticationProcessingFilter,-securityContextPersistenceFilter'],
	[pattern: '/**', filters:'JOINED_FILTERS,-restTokenValidationFilter,-restExceptionTranslationFilter']

]


grails.plugin.springsecurity.rest.logout.endpointUrl = '/api/logout'
grails.plugin.springsecurity.rest.login.failureStatusCode = 401

//token validate
grails.plugin.springsecurity.rest.token.validation.useBearerToken = true
grails.plugin.springsecurity.rest.token.validation.enableAnonymousAccess = true


grails.plugin.springsecurity.useSecurityEventListener = true
grails.plugin.springsecurity.onInteractiveAuthenticationSuccessEvent = { e, appCtx ->

	/*
	User.withTransaction {

		def user = User.findByIdAndIsDeleted(appCtx.springSecurityService.principal.id, false)
		if (!user.isAttached())// Checks whether the domain instance is attached to a currently active Hibernate session.
			user.attach()
		user.lastLoginDate = user.currentLoginTime // update last login date
		user.currentLoginDate = new Date() // update current login date
		user.save(flush: true, failOnError: true)

	}
	*/

	def utilsService = appCtx.utilsService
	utilsService?.setAppInitVars()

}

grails.plugin.springsecurity.onAbstractAuthenticationFailureEvent = { e, appCtx ->
	println "\nERROR auth failed for user $e.authentication.name: $e.exception.message\n"
}