#https://medium.com/@bfortuner/python-multithreading-vs-multiprocessing-73072ce5600b
from concurrent.futures import ProcessPoolExecutor, ThreadPoolExecutor

def multithreading(func, args, n_workers):
    with ThreadPoolExecutor(n_workers) as ex:
        #https://stackoverflow.com/a/6976926/4212158
        results = ex.map(lambda params: func(*params), args)
    return list(results)