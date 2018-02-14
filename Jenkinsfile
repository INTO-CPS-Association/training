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

      dir('examples'){
        sh "find . -name \"Tutorial*.tex\" -d 2 -execdir sh -c \'latexmk -pdf -bibtex -f `basename $1`\' \;"
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

