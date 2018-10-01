grails.plugin.springsecurity.userLookup.usernamePropertyName = 'username'

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
