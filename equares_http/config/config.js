module.exports = {
	development: {
        db: 'mongodb://localhost/equares-users',
	},
  	production: {
    	db: process.env.MONGOLAB_URI || process.env.MONGOHQ_URL,
 	}
}
