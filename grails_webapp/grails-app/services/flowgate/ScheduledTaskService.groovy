package flowgate

import grails.transaction.Transactional
import org.springframework.scheduling.annotation.Scheduled

@Transactional
class ScheduledTaskService {

    boolean lazyInit = false    // <--- this is important

//    @Scheduled(fixedRate = 5000L) // 5 seconds
    def myBusinessMethodForTheJob() {
        log.info 'Executing scheduled job...'
        println "executing scheduled job"
    }
}
