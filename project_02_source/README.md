#Init Database
flask --app auth_server.py db init
flask --app auth_server.py db migrate -m "Initial migration."
flask --app auth_server.py db upgrade

#Run
##Run Server
##Run UI Client