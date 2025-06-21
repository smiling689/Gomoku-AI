from flask import Flask, render_template, jsonify
from datetime import timedelta

app = Flask(__name__)
app.config['SEND_FILE_MAX_AGE_DEFAULT'] = timedelta(seconds=1)

@app.route('/')
def index():
    return render_template('index.html')

@app.route('/action/<x>+<y>', methods=['GET'])
def get_data(x,y):
    return jsonify([int(x)+1, int(y)+1])

if __name__ == '__main__':
    # app.run()
    app.run(debug=True)
