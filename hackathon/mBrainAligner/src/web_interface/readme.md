# Deploy mBrainAligner Demo on local machine (linux/Windows)

**Requirement: Python 3.x**
With the `server-code` pack, place it inside folder `mBrainAligner`, along with `binary`, `doc`, `examples` and `src`. 
**1. Set up the installation environment:**
```
pip install -r ./backend/requirements.txt
```
**2. Start backend services:**

```
cd backend
python app_padding.py
```
**3. Configure frontend:**
- Install `node.js`
	linux: run
	```
	sudo apt-get update
	sudo apt-get install -y nodejs
	```
	Windows: installation package can be found as `./setup-tools/node-v14.17.0-x86.msi`.
- Install `yarn`. Run
```
npm install -g yarn --registry=https://registry.npm.taobao.org
```
- Enable local debugging. Run
```
cd frontend
yarn install
yarn serve
```
**4. Alter relevant address information**
Redirect to `\frontend\src\views\Home.vue`, and change all `http://159.75.201.35` into your actual IP address.

To acquire the registration results of sample data, you may read this following section to [run mBrainAligner on sample data in batch](https://github.com/Vaa3D/vaa3d_tools/tree/master/hackathon/mBrainAligner#--run-mbrainaligner-on-sample-data-in-batch). 