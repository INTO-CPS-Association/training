node {
	try
  {

		
		stage ('Checkout'){
			checkout scm
		}

		// Mark the code build
		stage ('Build Examples'){

			dir('examples'){
				sh "latexmk -pdf -bibtex -f INTO-CPS_Examples_Compendium.tex"
			}
		}


    stage ('Build Tutorials'){
				dir('tutorials'){
					sh "sh build_tutorials.sh"
				}
    }


    stage ('Build Tutorials'){
        dir('methods'){
           sh "latexmk -pdf -bibtex -f INTO-CPS_Method_Guidelines.tex"
        }
    }

		stage ('Publish Artifactory'){

			if (env.BRANCH_NAME == 'development') {
			
			}
		}
	

	} catch (any) {
		currentBuild.result = 'FAILURE'
		throw any //rethrow exception to prevent the build from proceeding
	} finally {
  
		stage('Reporting'){

			// Notify on build failure using the Email-ext plugin
			emailext(body: '${DEFAULT_CONTENT}', mimeType: 'text/html',
							 replyTo: '$DEFAULT_REPLYTO', subject: '${DEFAULT_SUBJECT}',
							 to: emailextrecipients([[$class: 'CulpritsRecipientProvider'],
																			 [$class: 'RequesterRecipientProvider']]))
		}}
}

