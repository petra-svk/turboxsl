#include "template_task.h"

struct template_task_context_ {
    template_context *context;
    void (*function)(template_context *);
    shared_variable *workers;
};

void template_task_function(void *data)
{
    template_task_context *task_context = (template_task_context *)data;
    task_context->function(task_context->context);

    if (task_context->workers != NULL) shared_variable_decrease(task_context->workers);
}

void template_task_run(template_context *context, void (*function)(template_context *))
{
    if (context->context->pool == NULL)
    {
        function(context);
        return;
    }

    template_task_context *task_context = memory_allocator_new(sizeof(template_task_context));
    if (task_context == NULL)
    {
        error("template_task_run:: context");
        return;
    }

    task_context->context = context;
    task_context->function = function;
    // continue template task
    if (context->workers != NULL)
    {
        task_context->workers = context->workers;
        shared_variable_increase(task_context->workers);
    }

    threadpool_start(context->context->pool, template_task_function, task_context);
}

void template_task_run_and_wait(template_context *context, void (*function)(template_context *))
{
    if (context->context->pool == NULL)
    {
        function(context);
        return;
    }

    template_task_context *task_context = memory_allocator_new(sizeof(template_task_context));
    if (task_context == NULL)
    {
        error("template_task_run_and_wait:: context");
        return;
    }

    task_context->context = context;
    task_context->function = function;
    task_context->workers = shared_variable_create();

    threadpool_start(context->context->pool, template_task_function, task_context);

    shared_variable_wait(task_context->workers);
    shared_variable_release(task_context->workers);
}
