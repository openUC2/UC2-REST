import asyncio
from functools import wraps
# Borrowed from Wei: https://github.com/imjoy-team/imjoy-rpc/blob/015378cc5fa3310bfee7ec16237f19a9d1c461bf/python/imjoy_rpc/hypha/sync.py#L59
def convert_sync_to_async(sync_func, loop, executor):
    """Convert a synchronous function to an asynchronous function."""
    if asyncio.iscoroutinefunction(sync_func):
        return sync_func

    @wraps(sync_func)
    async def wrapped_async(*args, **kwargs):
        result_future = loop.create_future()
        def run_and_set_result():
            try:
                result = sync_func(*args, **kwargs)
                loop.call_soon_threadsafe(result_future.set_result, result)
            except Exception as e:
                loop.call_soon_threadsafe(result_future.set_exception, e)

        executor.submit(run_and_set_result)
        return await result_future

    return wrapped_async

def convert_async_to_sync(async_func, loop, executor):
    """Convert an asynchronous function to a synchronous function."""

    @wraps(async_func)
    def wrapped_sync(*args, **kwargs):
        async def async_wrapper():
            return await async_func(*args, **kwargs)

        future = asyncio.run_coroutine_threadsafe(async_wrapper(), loop)
        return future.result()

    return wrapped_sync

def _encode_callables(obj, wrap, loop, executor):
    """Encode callables in the given object to sync or async."""
    if isinstance(obj, dict):
        return {k: _encode_callables(v, wrap, loop, executor) for k, v in obj.items()}
    elif isinstance(obj, (list, tuple)):
        return [_encode_callables(item, wrap, loop, executor) for item in obj]
    elif callable(obj):
        return wrap(obj, loop, executor)
    else:
        return obj
