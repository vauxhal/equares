module.exports = {
	development: {
        db: 'mongodb://localhost/equares-users:3100',
	},
  	production: {
    	db: process.env.MONGOLAB_URI || process.env.MONGOHQ_URL,
 	}
}
