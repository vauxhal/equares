module.exports = {
	development: {
        db: 'mongodb://localhost/equares',
	},
  	production: {
    	db: process.env.MONGOLAB_URI || process.env.MONGOHQ_URL,
 	}
}
