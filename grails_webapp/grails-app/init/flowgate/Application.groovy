package flowgate

import grails.boot.GrailsApp
import grails.boot.config.GrailsAutoConfiguration
//import org.springframework.scheduling.annotation.EnableScheduling
//import grails.io.IOUtils

import org.springframework.beans.factory.config.YamlPropertiesFactoryBean
//import org.springframework.beans.factory.config.YamlMapFactoryBean
import org.springframework.context.EnvironmentAware
import org.springframework.core.env.Environment
//import org.springframework.core.env.MapPropertySource
import org.springframework.core.env.PropertiesPropertySource
import org.springframework.core.io.FileSystemResource
import org.springframework.core.io.Resource
//import org.springframework.scheduling.annotation.Scheduled


//TODO uncomment this for execution of scheduledTask in ScheduledTaskService
//@EnableScheduling
//class Application extends GrailsAutoConfiguration {
class Application extends GrailsAutoConfiguration implements EnvironmentAware {

    static void main(String[] args) {
        GrailsApp.run(Application, args)
    }

    @Override
    void setEnvironment(Environment environment) {
        String configPath = System.properties["local.config.location"]
        if(configPath){
            Resource resourceConfig = new FileSystemResource(configPath)
            if(resourceConfig.exists()){
                println "using external config: ${resourceConfig}"
                //  for yml config file use commented line / not working with mail config
                YamlPropertiesFactoryBean ypfb = new YamlPropertiesFactoryBean()
//                  YamlMapFactoryBean ypfb = new YamlMapFactoryBean()
                ypfb.setResources([resourceConfig] as Resource[])
                ypfb.afterPropertiesSet()
                Properties properties = ypfb.getObject()
                //  def config = new ConfigSlurper(grails.util.Environment.current.name).parse(IOUtils.toString(resourceConfig.getInputStream(), 'UTF-8'))
                //  def config = new ConfigSlurper(grails.util.Environment.current.name).parse(IOUtils.toString(resourceConfig.getInputStream(), 'UTF-8'))
                //  MapPropertySource source = new MapPropertySource(configPath, config)
                //  environment.propertySources.addFirst(source)
                //  environment.propertySources.addFirst(new PropertiesPropertySource('local.config.location', source))
                environment.propertySources.addFirst(new PropertiesPropertySource('local.config.location', properties))
//                println environment.propertySources.dump()
            }
        }

    }
}

